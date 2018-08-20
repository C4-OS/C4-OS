#include <c4alloc/c4alloc.h>
#include <c4rt/c4rt.h>
#include <c4rt/compiler.h>
#include <c4rt/stublibc.h>
#include <c4rt/elf.h>
#include <nameserver/nameserver.h>
#include <c4rt/interface/filesystem.h>
//#include <interfaces/pager.h>
//#include <c4/paging.h>
#include <c4/thread.h>
#include <stdint.h>
#include <stdbool.h>

typedef struct prognode {
	char name[FS_MAX_NAME_LEN + 1];
	struct prognode *next;
	fs_node_t node;
} prognode_t;

static c4a_heap_t progheap;

static inline bool strequal( const char *a, const char *b ){
	return strcmp(a, b) == 0;
}

prognode_t *enumerate_initprogs( unsigned fs ){
	prognode_t *ret  = NULL;
	prognode_t *head = NULL;
	fs_connection_t conn = {};
	fs_node_t rootdir = {};
	fs_node_t nodebuf = {};

	//fs_connect( fs, buffer, &conn );
	c4_debug_printf("--- initsys: connecting to server...\n");
	fs_connect(fs, &conn);
	c4_debug_printf("--- initsys: connected!\n");
	fs_get_root_dir(&conn, &rootdir);
	fs_get_root_dir(&conn, &rootdir);
	c4_debug_printf("--- initsys: got root dir...\n");
	fs_set_node(&conn, &rootdir);

	c4_debug_printf("--- initsys: looking for stuff...\n");
	if (fs_find_name(&conn, &nodebuf, "sbin", 4) > 0) {
		fs_set_node(&conn, &nodebuf);

		c4_debug_printf("--- initsys: got here...\n");
		fs_dirent_t dirbuf;
		while (fs_next_dirent(&conn, &dirbuf) > 0) {
			if (strequal(dirbuf.name, ".") || strequal(dirbuf.name, "..")) {
				continue;
			}

			fs_node_t foo;
			fs_dirent_to_node(&conn, &dirbuf, &foo);

			prognode_t *asdf = c4a_alloc(&progheap, sizeof(prognode_t));
			asdf->node = foo;
			asdf->next = NULL;

			strncpy(asdf->name, dirbuf.name, FS_MAX_NAME_LEN);

			if (head) {
				head->next = asdf;
				head       = head->next;

			} else {
				ret = head = asdf;
			}

			c4_debug_printf(
				"--- initsys: found \"%s\", inode %u, size: %u\n",
				dirbuf.name, foo.inode, foo.size);
		}

	} else {
		c4_debug_printf( "--- initsys: could not find /sbin, exiting...\n" );
	}

	fs_disconnect(&conn);
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

	//fs_connect( fs, buffer, &conn );
	fs_connect(fs, &conn);
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

int load_program( unsigned fs, prognode_t *prog, unsigned nameserver ){
	Elf32_Ehdr *foo = read_program( fs, prog );

	if ( foo ){
		c4_debug_printf(
			"--- initsys: read program, continuing to load \"%s\"\n",
			prog->name );

		char *args[] = { prog->name, NULL };
		char *env[]  = { "rootfs=/dev/ext2fs", NULL };
		elf_load(foo, args, env);

	} else {
		c4_debug_printf( "--- initsys: could not read file\n" );
	}

	return 0;
}

//void _start( uintptr_t nameserver ){
int main(int argc, char *argv[]) {
	// TODO: init stub in c4rt which takes care of common things like
	//       heap initialization
	c4a_heap_init( &progheap, 0xbeef0000 );
	c4_debug_printf( "--- initsys: hello, world! thread %u\n",
	                 c4_get_id());

	int nameserver = getnameserv();

	unsigned fs = 0;
	while (!fs) fs = nameserver_lookup( nameserver, "/dev/ext2fs" );

	c4_debug_printf( "--- initsys: have fs at %u\n", fs );

	prognode_t *nodes = enumerate_initprogs( fs );

	for ( prognode_t *temp = nodes; temp; temp = temp->next ){
		c4_debug_printf( "--- initsys: have inode %u, size: %u\n",
		                 temp->node.inode, temp->node.size );

		load_program( fs, temp, nameserver );
	}

	while ( true ){
		message_t msg;
		c4_msg_recieve( &msg, 1 );
	}

	c4_exit();
	return 0;
}
