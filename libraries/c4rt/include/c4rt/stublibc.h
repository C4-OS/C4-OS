#ifndef _C4RT_STUB_LIBC_H
#define _C4RT_STUB_LIBC_H 1
#include <c4rt/compiler.h>
#include <c4alloc/c4alloc.h>
#include <interfaces/filesystem.h>
#include <stddef.h>

// TODO: split all of this into their proper headers

enum {
	FILE_STATUS_PRETTY_GOOD,
	FILE_STATUS_END_OF_FILE,
	FILE_STATUS_ERROR,
};

enum {
	FILE_MODE_READ   = 1 << 1,
	FILE_MODE_WRITE  = 1 << 2,
	FILE_MODE_APPEND = 1 << 3,
};

// stdlib functions
void *malloc( size_t size );
void free( void *ptr );
void *calloc( size_t members, size_t size );
void realloc( void *ptr, size_t size );

// file functions
typedef struct c_filestruct {
	fs_connection_t connection;
	unsigned  server;
	unsigned  status;
	fs_node_t node;
	int       charbuf;
	char      mode;

	// TODO: do we need read/write offsets here?
} FILE;

FILE *fopen( const char *path, const char *mode );
FILE *fclose( FILE *fp );
FILE *freopen( FILE *fp );
size_t fread( void *ptr, size_t size, size_t members, FILE *fp );
size_t fwrite( const void *ptr, size_t size, size_t members, FILE *fp );
char *fgets( char *s, int size, FILE *stream );
int   fgetc( FILE *stream );
int   getc( FILE *stream );
int   getchar( void );
int   ungetc( int c, FILE *stream );

void clearerr( FILE *fp );
int feof( FILE *fp );
int ferror( FILE *fp );

// string functions
void *memcpy(void *dest, const void *src, size_t n);
void *memset(void *s, int c, size_t n);
size_t strlen(const char *s);
char *strcpy(char *dest, const char *src);
char *strncpy(char *dest, const char *src, size_t n);
size_t strlcpy(char *dest, const char *src, size_t size);
int strcmp( const char *s1, const char *s2 );
int strncmp( const char *s1, const char *s2, size_t n );
char *strchr( const char *s, int c );
size_t strcspn( const char *s, const char *reject );

// misc. function
char *getenv( const char *name );

// non-C functions
unsigned getnameserv( void );
c4a_heap_t *getc4heap( void );


#endif
