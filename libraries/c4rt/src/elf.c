#include <interfaces/pager.h>
#include <c4rt/elf.h>
#include <c4rt/c4rt.h>
#include <c4rt/stublibc.h>
#include <c4/paging.h>
#include <c4/thread.h>

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

// XXX: currently assumes that the returned page(s) will be mapped away
//      before the next call to allot_pages()
static inline void *allot_pages( unsigned pages ){
	void *ret = (void *)0xd0000000;

	ret = pager_request_pages( c4_get_pager(),
	                           (uintptr_t)ret,
	                           PAGE_READ | PAGE_WRITE,
	                           pages );

	return ret;
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

static inline uint8_t *elf_push_str( uint8_t **from,
                                     uint8_t **to,
                                     char     *str )
{
	size_t len = strlen( str ) + 1;

	*from -= len;
	*to   -= len;
	strcpy( (char *)*from, str );

	return *to;
}

static inline unsigned strlist_len( char **things ){
	unsigned ret = 0;

	for ( ; things[ret]; ret++ );

	return ret;
}

static inline char **elf_copy_strlist( uint8_t **from,
                                       uint8_t **to,
                                       char    **things )
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

static inline void setup_stack_params( uint8_t  **from,
                                       uint8_t  **to,
                                       unsigned nameserver,
                                       char     **argv,
                                       char     **envp )
{
	char **to_argv = elf_copy_strlist( from, to, argv );
	char **to_envp = elf_copy_strlist( from, to, envp );

	elf_push_arg( from, to, C4RT_INIT_MAGIC );
	elf_push_arg( from, to, (uintptr_t)to_envp );
	elf_push_arg( from, to, (uintptr_t)to_argv );
	elf_push_arg( from, to, nameserver );
	elf_push_arg( from, to, 0 );
}

//int elf_load( Elf32_Ehdr *elf, int nameserver ){
int elf_load( Elf32_Ehdr *elf, int nameserver, char **argv, char **envp ){
	unsigned stack_offset = 0xff0;

	void *entry      = (void *)elf->e_entry;
	void *to_stack   = (uint8_t *)0xa0000000;
	void *from_stack = (uint8_t *)allot_pages(1);
	//void *stack      = (uint8_t *)to_stack + stack_offset;

	uint8_t *to   = to_stack + stack_offset;
	uint8_t *from = from_stack + stack_offset;

	setup_stack_params( &from, &to, nameserver, argv, envp );

	int thread_id = c4_create_thread( entry, to,
	                                  THREAD_CREATE_FLAG_NEWMAP);

	c4_mem_grant_to( thread_id, from_stack, to_stack, 1,
	                 PAGE_READ | PAGE_WRITE );

	// load program headers
	for ( unsigned i = 0; i < elf->e_phnum; i++ ){
		Elf32_Phdr *header = elf_get_phdr( elf, i );
		uint8_t *progdata  = (uint8_t *)elf + header->p_offset;
		void    *addr      = (void *)header->p_vaddr;
		unsigned pages     = (header->p_memsz / PAGE_SIZE)
		                   + (header->p_memsz % PAGE_SIZE > 0);
		uint8_t *databuf   = allot_pages( pages );
		unsigned offset    = header->p_vaddr % PAGE_SIZE;
		void    *adjaddr   = (uint8_t *)addr - offset;

		for ( unsigned k = 0; k < header->p_filesz; k++ ){
			databuf[k + offset] = progdata[k];
		}

		// TODO: translate elf permissions into message permissions
		c4_mem_grant_to( thread_id, databuf, adjaddr, pages,
		                 PAGE_READ | PAGE_WRITE );
	}

	c4_set_pager( thread_id, c4_get_pager() );
	c4_continue_thread( thread_id );

	return thread_id;
}
