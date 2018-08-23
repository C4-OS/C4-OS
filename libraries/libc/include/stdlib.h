#ifndef _C4OS_LIBC_STDLIB_H
#define _C4OS_LIBC_STDLIB_H 1

#include <stddef.h>

void *malloc( size_t size );
void free( void *ptr );
void *calloc( size_t members, size_t size );
void realloc( void *ptr, size_t size );

char *getenv(const char *name);
void abort(void);
int atoi(const char *s);
int atexit(void (*func)(void));

#endif
