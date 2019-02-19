#ifndef _C4OS_LIBC_STDLIB_H
#define _C4OS_LIBC_STDLIB_H 1

#include <stddef.h>
#include <c4rt/elf.h>
#include <c4alloc/c4alloc.h>
#include <c4rt/addrman.h>

void c4rt_exit(int status);

void *c4rt_malloc( size_t size );
void c4rt_free( void *ptr );
void *c4rt_calloc( size_t members, size_t size );
void *c4rt_realloc( void *ptr, size_t size );

char *c4rt_getenv(const char *name);
void c4rt_abort(void);
int c4rt_atoi(const char *s);
int c4rt_atexit(void (*func)(void));

int c4rt_rand(void);
void c4rt_srand(unsigned int seed);
int c4rt_abs(int j);

// non-C functions, so no c4rt_* prefix needed
c4_process_t spawn(const char *name, const char *argv[], const char *envp[]);
unsigned getnameserv( void );
c4a_heap_t *getc4heap( void );
c4rt_vaddr_region_t *get_genregion(void);

#endif
