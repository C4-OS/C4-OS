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
#include <stdlib.h>

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

			strcpy(asdf->name, "/sbin/");
			strlcpy(asdf->name + 6, dirbuf.name, FS_MAX_NAME_LEN - 7);

			if (head) {
				head->next = asdf;
				head       = head->next;

			} else {
				ret = head = asdf;
			}

			c4_debug_printf(
				"--- initsys: found \"%s\", inode %u, size: %u\n",
				asdf->name, foo.inode, foo.size);
		}

	} else {
		c4_debug_printf( "--- initsys: could not find /sbin, exiting...\n" );
	}

	fs_disconnect(&conn);
	return ret;
}

int load_program( unsigned fs, prognode_t *prog, unsigned nameserver ){
	const char *args[] = { prog->name, NULL };
	const char *env[]  = { "rootfs=/dev/ext2fs", NULL };

	c4_debug_printf("--- initsys: loading \"%s\"\n", prog->name);
	spawn(prog->name, args, env);

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
