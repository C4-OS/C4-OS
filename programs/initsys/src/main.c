#include <c4rt/c4rt.h>
#include <c4rt/compiler.h>
#include <c4rt/stublibc.h>
#include <c4/paging.h>
#include <nameserver/nameserver.h>
#include <interfaces/filesystem.h>
#include <stdint.h>
#include <stdbool.h>

static uint8_t buffer[PAGE_SIZE] ALIGN_TO(PAGE_SIZE);

static inline bool strequal( const char *a, const char *b ){
	return strcmp(a, b) == 0;
}

void _start( uintptr_t nameserver ){
	c4_debug_printf( "--- initsys: hello, world! thread %u\n",
	                 c4_get_id());

	unsigned fs = 0;
	
	while ( !fs ){
		fs = nameserver_lookup( nameserver, "/dev/ext2fs" );
	}

	c4_debug_printf( "--- initsys: have fs at %u\n", fs );

	fs_node_t node;
	fs_connection_t conn = {};

	fs_connect( fs, buffer, &conn );

	fs_get_root_dir( fs, &node );
	fs_set_node( &conn, &node );

	fs_node_t nodebuf;
	if ( fs_find_name( &conn, &nodebuf, "sbin", 4 ) > 0 ){
		fs_set_node( &conn, &nodebuf );

		fs_dirent_t dirbuf;
		while ( fs_next_dirent( &conn, &dirbuf ) > 0 ){
			if ( strequal( dirbuf.name, "." ) || strequal( dirbuf.name, "." )){
				continue;
			}

			fs_node_t foo;
			fs_dirent_to_node( fs, &dirbuf, &foo );

			c4_debug_printf(
				"--- initsys: loading \"%s\", inode %u, size: %u\n",
				dirbuf.name, foo.inode, foo.size );
		}

	} else {
		c4_debug_printf( "--- initsys: could not find /sbin, exiting...\n" );
	}

	fs_disconnect( &conn );

	while ( true ){
		message_t msg;

		c4_msg_recieve( &msg, 0 );
	}

	c4_exit();
}
