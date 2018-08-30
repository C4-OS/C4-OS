#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <c4/paging.h>
#include <nameserver/nameserver.h>
#include <stdint.h>
#include <iso646.h>

static fs_node_t root_dir;
static fs_node_t current_dir;
static fs_connection_t current_fs;

static unsigned  root_server = 0;
//static uint8_t   fs_buffer[PAGE_SIZE] ALIGN_TO(PAGE_SIZE);

static inline void init_rootfs( void ){
	static bool initialized = false;

	if (initialized) {
		return;
	}

	initialized = true;
	char *fallback = "/dev/ext2fs";
	char *rootdev = getenv("rootfs");
	unsigned nameserver = getnameserv();

	c4_debug_printf("--- thread %u: nameserver at %u\n", c4_get_id(), nameserver);

	if (!rootdev) {
		c4_debug_printf("--- thread %u: could not find rootfs variable\n",
			c4_get_id());
		c4_debug_printf("--- thread %u: setting fallback: %s...\n",
			c4_get_id(), fallback);

		rootdev = fallback;
	}

	if ((root_server = nameserver_lookup(nameserver, rootdev)) == 0) {
		c4_debug_printf("--- thread %u: could not open rootfs \"%s\"\n",
			c4_get_id(), rootdev);
		return;
	}

	c4_debug_printf("--- thread %u: have server at %u\n", c4_get_id(), root_server);

	fs_connect(root_server, &current_fs);
	//fs_get_root_dir(root_server, &root_dir);
	fs_get_root_dir(&current_fs, &root_dir);
	current_dir = root_dir;

	c4_debug_printf("--- thread %u: got here\n", c4_get_id());
}

static unsigned translate_modeflags( const char *mode ){
	unsigned ret = 0;

	for (unsigned i = 0; mode[i]; i++) {
		switch (mode[i]) {
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
	fs_node_t temp = (*path == '/')? root_dir : current_dir;

	/*
	fs_connection_t conn = {};

	//fs_connect( root_server, fs_buffer, &conn );
	//fs_connect( root_server, fs_buffer, &conn );
	*/
	c4_debug_printf("--- fopen(): path: \"%s\"...\n", path);

	for ( int found = 1; found > 0; ){
		while ( *path == '/' ) path++;
		size_t foo = strcspn( path, "/" );
		size_t maxlen = sizeof( namebuf );

		if ( foo == 0 ){
			ret          = calloc( 1, sizeof( FILE ));
			//ret->server  = conn.server;
			ret->status  = FILE_STATUS_PRETTY_GOOD;
			ret->node    = temp;
			ret->charbuf = '\0';
			ret->mode    = translate_modeflags( mode );
			/*
			ret->conn    = (fs_connection_t){ };

			fs_set_connection_info( &ret->conn, &temp, conn.server );
			*/

			fs_connect(root_server, &ret->conn);
			fs_set_node(&ret->conn, &ret->node);
			break;
		}

		//fs_set_node(&conn, &temp);
		fs_set_node(&current_fs, &temp);

		strlcpy(namebuf, path, (foo + 1 < maxlen)? foo + 1 : maxlen);
		c4_debug_printf("--- fopen(): looking for \"%s\"...\n", namebuf);
		found = fs_find_name(&current_fs, &temp, namebuf, foo);
		path += foo;

		if (found < 0) {
			c4_debug_printf( "--- got error: %u\n", -found );
			break;
		}
	}

	//fs_disconnect( &conn );
	return ret;
}

int fclose( FILE *fp ){
	if (fp) {
		fp->status = FILE_STATUS_CLOSED;
		fs_disconnect(&fp->conn);
		free(fp);
	}

	return 0;
}

FILE *freopen( const char *path, const char *mode, FILE *fp ){
	FILE *temp = fopen( path, mode );

	if (temp) {
		fs_disconnect(&fp->conn);
		//fclose( fp );
		*fp = *temp;
		return fp;
	}

	return NULL;
}

DIR *opendir(const char *name){
	// TODO: check that it's actually a directory
	return fopen(name, "r");
}

struct dirent *readdir(DIR *dirp) {
	fs_dirent_t dirent;

	if (fs_next_dirent(&dirp->conn, &dirent) <= 0) {
		return NULL;
	}

	strlcpy(&dirp->dent.d_name, dirent.name, 256);
	dirp->dent.d_reclen = sizeof(struct dirent);
	dirp->dent.d_ino    = dirent.inode;
	dirp->dent.d_off    = 0;

	return &dirp->dent;
}

int closedir(DIR *dirp){
	return fclose(dirp);
}

size_t fread( void *ptr, size_t size, size_t members, FILE *fp ){
	//fs_connection_t conn = {};
	uint8_t *buffer = ptr;
	size_t len = size * members;

	size_t ret = 0;
	int nread = 0;

	while ((nread = fs_read_block(&fp->conn, buffer + ret, len)) > 0) {
		ret += nread;
		len -= nread;
	}

	return ret;
}

size_t fwrite( const void *ptr, size_t size, size_t members, FILE *fp );

char *fgets( char *s, int size, FILE *stream ){
	size_t n = 0;
	char c = fgetc( stream );

	while ( !feof(stream) && n < size - 1 ){
		s[n++] = c;

		if ( c == '\n' )
			break;

		c = fgetc( stream );
	}

	s[n] = '\0';
	return s;
}

int fgetc( FILE *stream ){
	if ( stream->status != FILE_STATUS_PRETTY_GOOD ){
		return -EBADF;
	}

	char c;
	size_t nread = fread( &c, 1, 1, stream );

	if ( nread > 0 ){
		return c;

	} else {
		stream->status = FILE_STATUS_END_OF_FILE;
		return EOF;
	}
}

int getc( FILE *stream ){
	return fgetc( stream );
}

int getchar( void );
int ungetc( int c, FILE *stream );

void clearerr( FILE *fp );

int feof( FILE *fp ){
	return fp->status == FILE_STATUS_END_OF_FILE;
}

int ferror( FILE *fp ){
	return fp->status != FILE_STATUS_PRETTY_GOOD;
}
