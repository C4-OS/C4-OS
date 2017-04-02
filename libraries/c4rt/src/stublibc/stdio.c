#include <c4rt/stublibc.h>
#include <c4/paging.h>
#include <nameserver/nameserver.h>
#include <stdint.h>
#include <iso646.h>

static fs_node_t root_dir;
static fs_node_t current_dir;
static unsigned  root_server = 0;
static uint8_t   fs_buffer[PAGE_SIZE] ALIGN_TO(PAGE_SIZE);

static inline void init_rootfs( void ){
	static bool initialized = false;

	if ( initialized ) {
		return;
	}

	initialized = true;
	char *rootdev = getenv( "rootfs" );
	unsigned nameserver = getnameserv();

	if ( !rootdev ){
		c4_debug_printf( "--- thread %u: could not find rootfs variable\n",
			c4_get_id());
		return;
	}

	if (( root_server = nameserver_lookup( nameserver, rootdev )) == 0 ){
		c4_debug_printf( "--- thread %u: could not open rootfs \"%s\"\n",
			c4_get_id(), rootdev );
		return;
	}

	fs_get_root_dir( root_server, &root_dir );
	current_dir = root_dir;
}

static unsigned translate_modeflags( const char *mode ){
	unsigned ret = 0;

	for ( unsigned i = 0; mode[i]; i++ ){
		switch ( mode[i] ){
			case 'r': ret |= FILE_MODE_READ;   break;
			case 'w': ret |= FILE_MODE_WRITE;  break;
			case '+': ret |= FILE_MODE_APPEND; break;
			default:  break;
		}
	}

	return ret;
}

FILE *fopen( const char *path, const char *mode ){
	init_rootfs();

	FILE *ret = NULL;
	char namebuf[FS_MAX_NAME_LEN + 1];
	fs_node_t temp = (*path == '/')? path++, root_dir : current_dir;
	fs_connection_t conn = {};

	fs_connect( root_server, fs_buffer, &conn );

	for ( int found = 1; found > 0; ){
		while ( *path == '/' ) path++;
		size_t foo = strcspn( path, "/" );
		size_t maxlen = sizeof( namebuf );

		if ( foo == 0 ){
			c4_debug_printf( "--- found node: %u\n", temp.inode );
			ret = malloc( sizeof( FILE ));
			ret->server  = conn.server;
			ret->status  = FILE_STATUS_PRETTY_GOOD;
			ret->node    = temp;
			ret->charbuf = '\0';
			ret->mode    = translate_modeflags( mode );
			break;
		}

		strlcpy( namebuf, path, (foo + 1 < maxlen)? foo + 1 : maxlen );
		fs_set_node( &conn, &temp );
		found = fs_find_name( &conn, &temp, namebuf, foo );
		path += foo;

		if ( found < 0 ){
			c4_debug_printf( "--- got error: %u\n", -found );
			break;
		}
	}

	fs_disconnect( &conn );

	return ret;
}

int fclose( FILE *fp ){
	fp->status = FILE_STATUS_CLOSED;
	return 0;
}

FILE *freopen( const char *path, const char *mode, FILE *fp ){
	FILE *temp = fopen( path, mode );

	if ( temp ){
		fclose( fp );
		*fp = *temp;
		return fp;
	}

	return NULL;
}

size_t fread( void *ptr, size_t size, size_t members, FILE *fp ){
	fs_connection_t conn = {};
	uint8_t *buffer = ptr;
	size_t len = size * members;

	fs_connect( fp->server, fs_buffer, &conn );
	fs_set_node( &conn, &fp->node );

	size_t ret = 0;
	int nread = 0;

	while (( nread = fs_read_block( &conn, buffer + ret, len )) > 0 ){
		ret += nread;
		len -= nread;
	}

	fs_disconnect( &conn );
	return ret;
}

size_t fwrite( const void *ptr, size_t size, size_t members, FILE *fp );
char *fgets( char *s, int size, FILE *stream );
int   fgetc( FILE *stream );
int   getc( FILE *stream );
int   getchar( void );
int   ungetc( int c, FILE *stream );
