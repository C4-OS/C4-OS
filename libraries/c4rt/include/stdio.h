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
	FILE_STATUS_PRETTY_GOOD,
	FILE_STATUS_CLOSED,
	FILE_STATUS_END_OF_FILE,
	FILE_STATUS_ERROR,
};

enum {
	FILE_MODE_READ   = 1 << 1,
	FILE_MODE_WRITE  = 1 << 2,
	FILE_MODE_APPEND = 1 << 3,
};

enum {
	EOF = -1,
};

// errno errors
enum {
	ENONE,
	EBADF,
};

// XXX: breaking the usual convention of enums for constant definitions
//      since some ports use the preprocessor to "detect" whether these are
//      defined and break anyway even if (it thinks) they aren't.
#define SEEK_SET 0
#define SEEK_END 1
#define SEEK_CUR 2

// buffer things
enum {
	BUFSIZ = 256
};

enum {
	_IOFBF,
	_IOLBF,
	_IONBF,
};

/*
enum {
	SEEK_SET,
};
*/

// TODO: move posix-y stuff to the proper headers
typedef uint32_t ino_t;

struct dirent {
	ino_t    d_ino;
	off_t    d_off;
	uint16_t d_reclen;
	uint8_t  d_type;

	char d_name[256];
};

// file functions
typedef struct c_filestruct {
	fs_connection_t conn;
	fs_node_t node;

	struct dirent dent;
	unsigned  status;
	int       charbuf;
	char      mode;
	bool      have_char;
	bool      used;
} FILE;

typedef FILE DIR;
DIR *opendir(const char *name);
struct dirent *readdir(DIR *dirp);
int closedir(DIR *dirp);

FILE *fopen( const char *path, const char *mode );
int   fclose( FILE *fp );
FILE *freopen( const char *path, const char *mode, FILE *fp );
size_t fread( void *ptr, size_t size, size_t members, FILE *fp );
char *fgets( char *s, int size, FILE *stream );
int   fgetc( FILE *stream );
int   getc( FILE *stream );
int   getchar( void );
int   ungetc( int c, FILE *stream );

size_t fwrite( const void *ptr, size_t size, size_t members, FILE *fp );
int fputc(int c, FILE *fp);
int fputs(const char *s, FILE *fp);

int    printf(const char *fmt, ...);
int   fprintf(FILE *fp, const char *fmt, ...);
int  vfprintf(FILE *fp, const char *fmt, va_list ap);

int fseek(FILE *fp, long offset, int whence);
int ftell(FILE *fp);
void rewind(FILE *fp);
void setbuf(FILE *stream, char *buf);

void clearerr( FILE *fp );
int feof( FILE *fp );
int ferror( FILE *fp );
int fflush(FILE *fp);

extern FILE *stderr;
extern FILE *stdin;
extern FILE *stdout;

#endif
