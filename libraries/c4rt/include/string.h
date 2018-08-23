#ifndef _C4OS_LIBC_STRING_H
#define _C4OS_LIBC_STRING_H 1

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

#endif
