#ifndef _C4OS_LIBC_STDIO_H
#define _C4OS_LIBC_STDIO_H 1

#include <c4rt/compiler.h>
#include <c4rt/c4rt.h>
#include <c4alloc/c4alloc.h>
#include <c4rt/interface/filesystem.h>
#include <stddef.h>
#include <stdarg.h>

enum {
	SEEK_SET,
};

// file functions
typedef struct c_filestruct {
	fs_connection_t conn;
	//unsigned  server;
	fs_node_t node;

	unsigned  status;
	int       charbuf;
	char      mode;
	bool      have_char;
	bool      used;
} FILE;

FILE *fopen( const char *path, const char *mode );
int   fclose( FILE *fp );
FILE *freopen( const char *path, const char *mode, FILE *fp );
size_t fread( void *ptr, size_t size, size_t members, FILE *fp );
size_t fwrite( const void *ptr, size_t size, size_t members, FILE *fp );
char *fgets( char *s, int size, FILE *stream );
int   fgetc( FILE *stream );
int   getc( FILE *stream );
int   getchar( void );
int   ungetc( int c, FILE *stream );

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

#endif
