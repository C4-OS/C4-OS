#include <c4alloc/c4alloc.h>
#include <c4rt/c4rt.h>
#include <c4rt/compiler.h>
#include <c4rt/stublibc.h>
#include <c4rt/elf.h>
#include <nameserver/nameserver.h>
#include <interfaces/filesystem.h>
#include <interfaces/pager.h>
#include <c4/paging.h>
#include <c4/thread.h>
#include <stdint.h>
#include <stdbool.h>

typedef struct prognode {
	struct prognode *next;
	fs_node_t node;
} prognode_t;

static c4a_heap_t progheap;
static uint8_t buffer[PAGE_SIZE] ALIGN_TO(PAGE_SIZE);

static inline bool strequal( const char *a, const char *b ){
	return strcmp(a, b) == 0;
}

prognode_t *enumerate_initprogs( unsigned fs ){
	prognode_t *ret  = NULL;
	prognode_t *head = NULL;
	fs_connection_t conn = {};
	fs_node_t rootdir = {};
	fs_node_t nodebuf = {};

	fs_connect( fs, buffer, &conn );
	fs_get_root_dir( conn.server, &rootdir );
	fs_set_node( &conn, &rootdir );

	if ( fs_find_name( &conn, &nodebuf, "sbin", 4 ) > 0 ){
		fs_set_node( &conn, &nodebuf );

		fs_dirent_t dirbuf;
		while ( fs_next_dirent( &conn, &dirbuf ) > 0 ){
			if ( strequal( dirbuf.name, "." ) || strequal( dirbuf.name, ".." )){
				continue;
			}

			fs_node_t foo;
			fs_dirent_to_node( conn.server, &dirbuf, &foo );

			prognode_t *asdf = c4a_alloc( &progheap, sizeof( prognode_t ));
			asdf->node = foo;
			asdf->next = NULL;

			if ( head ){
				head->next = asdf;
				head       = head->next;

			} else {
				ret = head = asdf;
			}

			c4_debug_printf(
				"--- initsys: found \"%s\", inode %u, size: %u\n",
				dirbuf.name, foo.inode, foo.size );
		}

	} else {
		c4_debug_printf( "--- initsys: could not find /sbin, exiting...\n" );
	}

	fs_disconnect( &conn );
	return ret;
}

Elf32_Ehdr *read_program( unsigned fs, prognode_t *prog ){
	// XXX: fixed-size buffer to store elfs from files, this will limit the size
	//      of executables which can be loaded, and wastes memory
	//
	// TODO: dynamically allocate/free file buffers, or read from elf directly
	//       into memory for the new process (which will require being able to
	//       set the byte location of a file stream)
	static uint8_t progbuf[PAGE_SIZE * 128] ALIGN_TO(PAGE_SIZE);
	fs_connection_t conn = {};
	void *ret = NULL;

	if ( prog->node.size >= sizeof(progbuf) ){
		// can't read the whole file into the buffer, so just return NULL
		// to signal an error
		goto done;
	}

	fs_connect( fs, buffer, &conn );
	fs_set_node( &conn, &prog->node );

	size_t i = 0;
	int nread = 0;

	while (( nread = fs_read_block( &conn, progbuf + i, PAGE_SIZE )) > 0 ){
		i += nread;
	}

	ret = elf_is_valid( (Elf32_Ehdr*)progbuf )? progbuf : NULL;

done:
	fs_disconnect( &conn );
	return ret;
}

static void *allot_pages( unsigned pages ){
	void *ret = (void *)0xd0000000;

	ret = pager_request_pages( c4_get_pager(),
	                           (uintptr_t)ret,
	                           PAGE_READ | PAGE_WRITE,
	                           pages );

	return ret;
}

static inline void elf_load_set_arg( uint8_t *stack,
                                     unsigned offset,
                                     unsigned arg,
                                     unsigned value )
{
	*((unsigned *)(stack + offset) + arg + 1) = value;
}

int elf_load( Elf32_Ehdr *elf, int nameserver ){
	unsigned stack_offset = 0xff8;

	void *entry      = (void *)elf->e_entry;
	void *to_stack   = (uint8_t *)0xa0000000;
	void *from_stack = (uint8_t *)allot_pages(1);
	void *stack      = (uint8_t *)to_stack + stack_offset;

	// copy the output info to the new stack
	elf_load_set_arg( from_stack, stack_offset, 0, nameserver );

	int thread_id = c4_create_thread( entry, stack,
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

int load_program( unsigned fs, prognode_t *prog, unsigned nameserver ){
	Elf32_Ehdr *foo = read_program( fs, prog );

	if ( foo ){
		c4_debug_printf( "--- initsys: seems legit, continuing to load...\n" );
		elf_load( foo, nameserver );

	} else {
		c4_debug_printf( "--- initsys: could not read file\n" );
	}

	return 0;
}

void _start( uintptr_t nameserver ){
	// TODO: init stub in c4rt which takes care of common things like
	//       heap initialization
	c4a_heap_init( &progheap, 0xbeef0000 );
	c4_debug_printf( "--- initsys: hello, world! thread %u\n",
	                 c4_get_id());

	unsigned fs = 0;
	while ( !fs ) fs = nameserver_lookup( nameserver, "/dev/ext2fs" );

	c4_debug_printf( "--- initsys: have fs at %u\n", fs );

	prognode_t *nodes = enumerate_initprogs( fs );

	for ( prognode_t *temp = nodes; temp; temp = temp->next ){
		c4_debug_printf( "--- initsys: have inode %u, size: %u\n",
		                 temp->node.inode, temp->node.size );

		load_program( fs, temp, nameserver );
	}

	while ( true ){
		message_t msg;
		c4_msg_recieve( &msg, 0 );
	}

	c4_exit();
}
