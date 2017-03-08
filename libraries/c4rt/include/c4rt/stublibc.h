#ifndef _C4RT_STUB_LIBC_H
#define _C4RT_STUB_LIBC_H 1
#include <c4rt/compiler.h>
#include <stddef.h>

void *memcpy(void *dest, const void *src, size_t n);
void *memset(void *s, int c, size_t n);
size_t strlen(const char *s);
char *strcpy(char *dest, const char *src);
char *strncpy(char *dest, const char *src, size_t n);
size_t strlcpy(char *dest, const char *src, size_t size);
int strcmp( const char *s1, const char *s2 );

#endif
