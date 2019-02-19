#ifndef _C4OS_LIBC_STDIO_H
#define _C4OS_LIBC_STDIO_H 1

#include <c4rt/compiler.h>
#include <c4rt/c4rt.h>
#include <c4alloc/c4alloc.h>
#include <c4rt/interface/filesystem.h>
#include <stddef.h>
#include <stdarg.h>
#include <sys/types.h>

enum {
	C4RT_FILE_STATUS_PRETTY_GOOD,
	C4RT_FILE_STATUS_CLOSED,
	C4RT_FILE_STATUS_END_OF_FILE,
	C4RT_FILE_STATUS_ERROR,
};

enum {
	C4RT_FILE_MODE_READ   = 1 << 1,
	C4RT_FILE_MODE_WRITE  = 1 << 2,
	C4RT_FILE_MODE_APPEND = 1 << 3,
};

enum {
	C4RT_EOF = -1,
};

// errno errors
enum {
	C4RT_ENONE,
	C4RT_EBADF,
};

enum {
	C4RT_SEEK_SET = 0,
	C4RT_SEEK_END = 1,
	C4RT_SEEK_CUR = 2,
};

// TODO: move posix-y stuff to the proper headers
typedef uint32_t c4rt_ino_t;

struct c4rt_dirent {
	c4rt_ino_t d_ino;
	c4rt_off_t d_off;
	uint16_t   d_reclen;
	uint8_t    d_type;

	char d_name[256];
};

// file functions
typedef struct c4rt_filestruct {
	fs_connection_t conn;
	fs_node_t node;

	struct c4rt_dirent dent;
	unsigned  status;
	int       charbuf;
	char      mode;
	bool      have_char;
	bool      used;
} c4rt_file_t;

typedef c4rt_file_t c4rt_dir_t;
c4rt_dir_t *c4rt_opendir(const char *name);
struct c4rt_dirent *c4rt_readdir(c4rt_dir_t *dirp);
int c4rt_closedir(c4rt_dir_t *dirp);

c4rt_file_t *c4rt_fopen( const char *path, const char *mode );
c4rt_file_t *c4rt_freopen( const char *path, const char *mode, c4rt_file_t *fp );
size_t c4rt_fread( void *ptr, size_t size, size_t members, c4rt_file_t *fp );
size_t c4rt_fwrite( const void *ptr, size_t size, size_t members, c4rt_file_t *fp );
char *c4rt_fgets( char *s, int size, c4rt_file_t *stream );

int c4rt_fclose( c4rt_file_t *fp );
int c4rt_fgetc( c4rt_file_t *stream );
int c4rt_getc( c4rt_file_t *stream );
int c4rt_getchar( void );
int c4rt_ungetc( int c, c4rt_file_t *stream );
int c4rt_fputc(int c, c4rt_file_t *fp);
int c4rt_fputs(const char *s, c4rt_file_t *fp);

int c4rt_printf(const char *fmt, ...);
int c4rt_fprintf(c4rt_file_t *fp, const char *fmt, ...);
int c4rt_vfprintf(c4rt_file_t *fp, const char *fmt, va_list ap);

int c4rt_fseek(c4rt_file_t *fp, long offset, int whence);
int c4rt_ftell(c4rt_file_t *fp);
void c4rt_rewind(c4rt_file_t *fp);

void c4rt_clearerr(c4rt_file_t *fp);
int c4rt_feof(c4rt_file_t *fp);
int c4rt_ferror(c4rt_file_t *fp);
int c4rt_fflush(c4rt_file_t *fp);

extern c4rt_file_t *stderr;
extern c4rt_file_t *stdin;
extern c4rt_file_t *stdout;

#endif
