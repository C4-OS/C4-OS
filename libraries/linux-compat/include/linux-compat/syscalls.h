#ifndef _C4OS_LINUX_COMPAT_SYSCALLS_H
#define _C4OS_LINUX_COMPAT_SYSCALLS_H 1

enum {
	LINUX_COMPAT_MAX_SYSCALLS = 128,
};

typedef long (*compat_syscall_handler)(long a1, long a2, long a3,
                                       long a4, long a5, long a6);

// TODO: maybe have syscall_N functions for different numbers of arguments,
//       overhead is probably minimal though
long linux_compat_syscall(long n, long a1, long a2, long a3,
                                  long a4, long a5, long a6);

#endif
