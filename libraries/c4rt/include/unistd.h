#ifndef _C4OS_LIBC_UNISTD_H
#define _C4OS_LIBC_UNISTD_H 1

#include <sys/types.h>
#include <stdint.h>

typedef uint32_t useconds_t;
int usleep(useconds_t useconds);
unsigned int sleep(unsigned int seconds);

#endif
