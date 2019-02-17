#ifndef _C4OS_LIBC_STDLIB_H
#define _C4OS_LIBC_STDLIB_H 1

#include <stddef.h>
#include <c4rt/elf.h>
#include <c4alloc/c4alloc.h>
#include <c4rt/addrman.h>

enum {
	RAND_MAX = 2147483647,
};

void exit(int status);

void *malloc( size_t size );
void free( void *ptr );
void *calloc( size_t members, size_t size );
void *realloc( void *ptr, size_t size );

char *getenv(const char *name);
void abort(void);
int atoi(const char *s);
int atexit(void (*func)(void));

int rand(void);
void srand(unsigned int seed);
int abs(int j);

// non-C functions
c4_process_t spawn(const char *name, const char *argv[], const char *envp[]);
unsigned getnameserv( void );
c4a_heap_t *getc4heap( void );
c4rt_vaddr_region_t *get_genregion(void);

#endif
