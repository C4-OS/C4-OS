#include <c4rt/c4rt.h>
#include <c4rt/ringbuffer.h>
#include <c4rt/compiler.h>
#include <c4rt/stublibc.h>
#include <nameserver/nameserver.h>
#include <interfaces/filesystem.h>
#include <c4/paging.h>
#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

static uint8_t buffer[PAGE_SIZE] ALIGN_TO(PAGE_SIZE);
static char *types[] = { "unknown", "file", "directory", "symlink", };

void _start( unsigned nameserver ){
	unsigned fs = 9;
	fs_node_t node;
	message_t msg;

	/*
	while ( !fs ){
		fs = nameserver_lookup( nameserver, "/dev/fs" );
	}
	*/

	c4_debug_printf( "--- fstest: have fs server at %u\n", fs );

	c4_msg_recieve( &msg, 0 );
	fs_get_root_dir( fs, &node );

	fs_dirent_t dirbuf;
	fs_connection_t conn;

	fs_connect( fs, buffer, &conn );
	fs_set_node( &conn, &node );

	while ( fs_next_dirent( &conn, &dirbuf ) > 0 ){
		c4_debug_printf( "--- fstest: found \"%s\"\n", dirbuf.name );

		if ( strcmp( dirbuf.name, "boot" ) == 0 ){
			c4_debug_printf( "--- fstest: ^ found this\n" );
		}
	}

	fs_node_t foo;
	char *fooname = "foo.sh";

	foo.inode = 0;

	if ( fs_find_name( &conn, &foo, fooname, strlen(fooname) )){
		c4_debug_printf( "--- fstest: found %s, inode: %u\n",
			fooname, foo.inode );
	}

	fs_disconnect( &conn );
	c4_debug_printf( "--- fstest: disconnected from server %u\n", fs );

	c4_exit();
}
