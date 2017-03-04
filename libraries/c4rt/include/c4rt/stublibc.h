#ifndef _C4RT_STUB_LIBC_H
#define _C4RT_STUB_LIBC_H 1
#include <c4rt/compiler.h>
#include <stddef.h>

WEAK void *memcpy(void *dest, const void *src, size_t n);
WEAK void *memset(void *s, int c, size_t n);
WEAK size_t strlen(const char *s);
WEAK char *strcpy(char *dest, const char *src);
WEAK char *strncpy(char *dest, const char *src, size_t n);
WEAK size_t strlcpy(char *dest, const char *src, size_t size);

#endif
