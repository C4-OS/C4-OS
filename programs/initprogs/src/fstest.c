#include <c4rt/c4rt.h>
#include <c4rt/ringbuffer.h>
//#include <c4rt/compiler.h>
#include <c4rt/stublibc.h>
#include <nameserver/nameserver.h>
#include <interfaces/filesystem.h>
#include <c4/paging.h>
#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

static uint8_t buffer[PAGE_SIZE] ALIGN_TO(PAGE_SIZE);
static char *types[] = { "unknown", "file", "directory", "symlink", };

// XXX: compiler emits a 'call $0x0' instruction if strcmp() isn't defined
//      in the current file scope, maybe it's a compiler bug?
int strcmp( const char *s1, const char *s2 ){
	for ( ; *s1 && *s2; s1++, s2++ ){
		if ( *s1 == *s2 ) continue;
		if ( *s1 <  *s2 ) return -1;
		if ( *s1 >  *s2 ) return 1;
	}

	return 0;
}

void _start( unsigned nameserver ){
	c4_debug_printf( "--- fstest: got here\n" );
	unsigned fs = 9;

	/*
	while ( !fs ){
		fs = nameserver_lookup( nameserver, "/dev/fs" );
	}
	*/

	c4_debug_printf( "--- fstest: have fs server at %u\n", fs );

	fs_node_t node;
	message_t msg;

	c4_msg_recieve( &msg, 0 );
	fs_get_root_dir( fs, &node );
	c4_debug_printf( "--- fstest: root inode of type %s at %u, size: %u\n",
		types[node.type], node.inode, node.size );

	fs_dirent_t dirbuf;
	fs_connection_t conn;

	fs_connect( fs, buffer, &conn );
	c4_debug_printf( "--- fstest: connected to server %u\n", fs );
	fs_set_node( &conn, &node );
	c4_debug_printf( "--- fstest: set current node\n" );

	while ( fs_next_dirent( &conn, &dirbuf ) > 0 ){
		c4_debug_printf( "--- fstest: found \"%s\"\n", dirbuf.name );

		if ( strcmp( dirbuf.name, "boot" ) == 0 ){
			c4_debug_printf( "--- fstest: ^ found this\n" );
		}
	}

	fs_disconnect( &conn );
	c4_debug_printf( "--- fstest: disconnected from server %u\n", fs );

	c4_exit();
}
