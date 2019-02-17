#ifndef _C4OS_LIBC_ERRNO_H
#define _C4OS_LIBC_ERRNO_H 1

extern int errno;

char *strerror(int errnum);
void perror(const char *s);

#endif
