#include <c4rt/interface/pager.h>
#include <c4rt/elf.h>
#include <c4rt/c4rt.h>
#include <c4/paging.h>
#include <c4/thread.h>
#include <c4/bootinfo.h>

#include <string.h>

bool elf_is_valid( Elf32_Ehdr *elf ){
	return elf->e_ident[0] == ELFMAG0
	    && elf->e_ident[1] == ELFMAG1
	    && elf->e_ident[2] == ELFMAG2
	    && elf->e_ident[3] == ELFMAG3;
}

Elf32_Shdr *elf_get_shdr( Elf32_Ehdr *, unsigned );
Elf32_Shdr *elf_get_shdr_byname( Elf32_Ehdr *, char * );

Elf32_Phdr *elf_get_phdr( Elf32_Ehdr *elf, unsigned index ){
	Elf32_Phdr *ret = NULL;

	if ( index < elf->e_phnum ){
		uintptr_t temp = (uintptr_t)elf + elf->e_phoff;
		temp += elf->e_phentsize * index;

		ret = (Elf32_Phdr *)temp;
	}

	return ret;
}

Elf32_Sym  *elf_get_sym( Elf32_Ehdr *, int, char * );
char       *elf_get_sym_name( Elf32_Ehdr *, Elf32_Sym *, char * );
Elf32_Sym  *elf_get_sym_byname( Elf32_Ehdr *, char *, char * );

static int32_t allot_pages( unsigned pages ){
	return pager_request_pages( C4_PAGER,
	                            PAGE_READ | PAGE_WRITE,
	                            pages );
}

static inline uint8_t *elf_push_arg( uint8_t **from,
                                     uint8_t **to,
                                     uintptr_t value )
{
	*from -= sizeof(uintptr_t);
	*to   -= sizeof(uintptr_t);

	*(uintptr_t *)*from = value;

	return *to;
}

static inline uint8_t *elf_push_str( uint8_t    **from,
                                     uint8_t    **to,
                                     const char *str )
{
	size_t len = strlen( str ) + 1;

	*from -= len;
	*to   -= len;
	strcpy( (char *)*from, str );

	return *to;
}

static inline unsigned strlist_len( const char **things ){
	unsigned ret = 0;

	for ( ; things[ret]; ret++ );

	return ret;
}

static inline char **elf_copy_strlist( uint8_t    **from,
                                       uint8_t    **to,
                                       const char **things )
{
	unsigned len = strlist_len( things );
	uint8_t *foo[len];

	for ( unsigned i = 0; i < len; i++ ){
		foo[i] = elf_push_str( from, to, things[i] );
	}

	elf_push_arg( from, to, 0 );

	for ( unsigned i = len; i; i-- ){
		elf_push_arg( from, to, (uintptr_t)foo[i - 1] );
	}

	return (char **)*to;
}

static inline void setup_stack_params( uint8_t    **from,
                                       uint8_t    **to,
                                       unsigned   nameserver,
                                       const char **argv,
                                       const char **envp )
{
	char **to_argv = elf_copy_strlist( from, to, argv );
	char **to_envp = elf_copy_strlist( from, to, envp );

	elf_push_arg( from, to, C4RT_INIT_MAGIC );
	elf_push_arg( from, to, (uintptr_t)to_envp );
	elf_push_arg( from, to, (uintptr_t)to_argv );
	elf_push_arg( from, to, nameserver );
	elf_push_arg( from, to, 0 );
}

c4_process_t elf_load(Elf32_Ehdr *elf, const char **argv, const char **envp){
	return elf_load_full(elf, allot_pages, C4_NAMESERVER, C4_PAGER, argv, envp);
}

c4_process_t elf_load_full( Elf32_Ehdr *elf,
                            int32_t (*page_allot)(unsigned),
                            int nameserver,
                            int pager,
                            const char **argv,
                            const char **envp )
{
	c4_process_t ret;

	unsigned stack_offset = 0xfe0;
	int frame = page_allot(1);
	int cspace = c4_cspace_create();
	int aspace = c4_addrspace_create();
	C4_ASSERT(cspace > 0);
	C4_ASSERT(aspace > 0);

	void *entry          = (void *)elf->e_entry;
	uintptr_t to_stack   = 0xa0000000;
	uintptr_t from_stack = 0xda000000;

	int c4ret = 0;
	int msgq = c4_msg_create_sync();

	c4_cspace_copy(C4_CURRENT_CSPACE, msgq,         cspace, C4_SERV_PORT);
	c4_cspace_copy(C4_CURRENT_CSPACE, aspace,       cspace, C4_CURRENT_ADDRSPACE);
	c4_cspace_copy(C4_CURRENT_CSPACE, C4_BOOT_INFO, cspace, C4_BOOT_INFO);
	c4_cspace_copy(C4_CURRENT_CSPACE, pager,        cspace, C4_PAGER);

	if (nameserver) {
		c4_cspace_copy(C4_CURRENT_CSPACE, nameserver, cspace, C4_NAMESERVER);
		c4_cspace_restrict(cspace, C4_NAMESERVER,
		                   CAP_MODIFY | CAP_SHARE | CAP_MULTI_USE);
	}

	// map the bootinfo structure
	// TODO: possibly make bootinfo mapping done on-demand, but this is fine
	c4ret = c4_addrspace_map(aspace, C4_BOOT_INFO, (uintptr_t)BOOTINFO_ADDR, PAGE_READ);
	C4_ASSERT(c4ret >= 0);

	c4ret = c4_addrspace_map(C4_CURRENT_ADDRSPACE, frame, from_stack,
	                         PAGE_READ | PAGE_WRITE);
	C4_ASSERT(c4ret >= 0);
	c4ret = c4_addrspace_map(aspace, frame, to_stack, PAGE_READ | PAGE_WRITE);
	C4_ASSERT(c4ret >= 0);

	uint8_t *from = (void *)(from_stack + stack_offset);
	uint8_t *to   = (void *)(to_stack   + stack_offset);
	setup_stack_params(&from, &to, C4_NAMESERVER, argv, envp);
	c4ret = c4_addrspace_unmap(C4_CURRENT_ADDRSPACE, from_stack);
	C4_ASSERT(c4ret >= 0);

	int thread_id = c4_create_thread(entry, to, 0);
	c4_debug_printf("--- elf_load(): made thread %u\n", thread_id);
	C4_ASSERT(thread_id > 0);

	c4_set_addrspace(thread_id, aspace);
	c4_set_capspace(thread_id, cspace);
	c4_set_pager(thread_id, pager);

	ret.addrspace = aspace;
	ret.capspace  = cspace;
	ret.thread    = thread_id;
	ret.endpoint  = msgq;

	// TODO: initialized .bss

	// load program headers
	for (unsigned i = 0; i < elf->e_phnum; i++) {
		Elf32_Phdr *header = elf_get_phdr(elf, i);
		uint8_t *progdata  = (uint8_t *)elf + header->p_offset;
		uintptr_t addr     = header->p_vaddr;
		unsigned pages     = (header->p_memsz / PAGE_SIZE)
		                   + (header->p_memsz % PAGE_SIZE > 0);
		int frame          = page_allot(pages);
		uintptr_t databuf  = 0xda000000;
		uint8_t *dataptr   = (void *)databuf;
		unsigned offset    = header->p_vaddr % PAGE_SIZE;
		uintptr_t adjaddr  = addr - offset;

		// TODO: translate elf permissions into message permissions
		c4_addrspace_map(C4_CURRENT_ADDRSPACE, frame, databuf,
		                 PAGE_READ | PAGE_WRITE);
		c4_addrspace_map( aspace, frame, adjaddr, PAGE_READ | PAGE_WRITE );

		for (unsigned k = 0; k < header->p_filesz; k++) {
			dataptr[k + offset] = progdata[k];
		}

		c4_addrspace_unmap(C4_CURRENT_ADDRSPACE, databuf);
		c4_cspace_move(C4_CURRENT_CSPACE, frame, cspace,
		               i + C4_DEFAULT_OBJECT_END);
	}

	c4_continue_thread(thread_id);
	return ret;
}
