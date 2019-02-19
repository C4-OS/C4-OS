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
	char *rootdev = c4rt_getenv("rootfs");
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
			case 'r': ret |= C4RT_FILE_MODE_READ;   break;
			case 'w': ret |= C4RT_FILE_MODE_WRITE;  break;
			case '+': ret |= C4RT_FILE_MODE_APPEND; break;
			default:  break;
		}
	}

	return ret;
}

c4rt_file_t *c4rt_fopen( const char *path, const char *mode ){
	init_rootfs();

	c4rt_file_t *ret = NULL;
	char namebuf[FS_MAX_NAME_LEN + 1];
	fs_node_t temp = (*path == '/')? root_dir : current_dir;

	/*
	fs_connection_t conn = {};

	//fs_connect( root_server, fs_buffer, &conn );
	//fs_connect( root_server, fs_buffer, &conn );
	*/
	c4_debug_printf("--- c4rt_fopen(): path: \"%s\"...\n", path);

	for ( int found = 1; found > 0; ){
		while ( *path == '/' ) path++;
		size_t foo = strcspn( path, "/" );
		size_t maxlen = sizeof( namebuf );

		if ( foo == 0 ){
			ret          = c4rt_calloc(1, sizeof(c4rt_file_t));
			//ret->server  = conn.server;
			ret->status  = C4RT_FILE_STATUS_PRETTY_GOOD;
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
		c4_debug_printf("--- c4rt_fopen(): looking for \"%s\"...\n", namebuf);
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

int c4rt_fclose(c4rt_file_t *fp) {
	if (fp) {
		fp->status = C4RT_FILE_STATUS_CLOSED;
		fs_disconnect(&fp->conn);
		c4rt_free(fp);
	}

	return 0;
}

c4rt_file_t *c4rt_freopen(const char *path, const char *mode, c4rt_file_t *fp) {
	c4rt_file_t *temp = c4rt_fopen( path, mode );

	if (temp) {
		fs_disconnect(&fp->conn);
		//fclose( fp );
		*fp = *temp;
		return fp;
	}

	return NULL;
}

c4rt_dir_t *c4rt_opendir(const char *name){
	// TODO: check that it's actually a directory
	return c4rt_fopen(name, "r");
}

struct c4rt_dirent *c4rt_readdir(c4rt_dir_t *dirp) {
	fs_dirent_t dirent;

	if (fs_next_dirent(&dirp->conn, &dirent) <= 0) {
		return NULL;
	}

	strlcpy(dirp->dent.d_name, dirent.name, 256);
	dirp->dent.d_reclen = sizeof(struct c4rt_dirent);
	dirp->dent.d_ino    = dirent.inode;
	dirp->dent.d_off    = 0;

	return &dirp->dent;
}

int c4rt_closedir(c4rt_dir_t *dirp){
	return c4rt_fclose(dirp);
}

size_t c4rt_fread(void *ptr, size_t size, size_t members, c4rt_file_t *fp) {
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

size_t c4rt_fwrite(const void *ptr, size_t size,
                   size_t members, c4rt_file_t *fp)
{
	//fs_connection_t conn = {};
	const uint8_t *buffer = ptr;
	size_t len = size * members;

	size_t ret = 0;
	int nread = 0;

	while ((nread = fs_write_block(&fp->conn, buffer + ret, len)) > 0) {
		ret += nread;
		len -= nread;
	}

	return ret;
}

int c4rt_fputc(int c, c4rt_file_t *fp){
	unsigned char wr = c;
	c4rt_fwrite(&wr, 1, 1, fp);
	return c;
}

int c4rt_fputs(const char *s, c4rt_file_t *fp){
	return c4rt_fwrite(s, strlen(s), 1, fp);
}

char *c4rt_fgets( char *s, int size, c4rt_file_t *stream ){
	size_t n = 0;
	char c = c4rt_fgetc( stream );

	while ( !c4rt_feof(stream) && n < size - 1 ){
		s[n++] = c;

		if ( c == '\n' )
			break;

		c = c4rt_fgetc( stream );
	}

	s[n] = '\0';
	return s;
}

int c4rt_fgetc( c4rt_file_t *stream ){
	if ( stream->status != C4RT_FILE_STATUS_PRETTY_GOOD ){
		return -C4RT_EBADF;
	}

	char c;
	size_t nread = c4rt_fread( &c, 1, 1, stream );

	if ( nread > 0 ){
		return c;

	} else {
		stream->status = C4RT_FILE_STATUS_END_OF_FILE;
		return C4RT_EOF;
	}
}

int c4rt_getc( c4rt_file_t *stream ){
	return c4rt_fgetc( stream );
}

int c4rt_getchar( void );
int c4rt_ungetc( int c, c4rt_file_t *stream );

void c4rt_clearerr( c4rt_file_t *fp );

int c4rt_feof( c4rt_file_t *fp ){
	return fp->status == C4RT_FILE_STATUS_END_OF_FILE;
}

int c4rt_ferror( c4rt_file_t *fp ){
	return fp->status != C4RT_FILE_STATUS_PRETTY_GOOD;
}
