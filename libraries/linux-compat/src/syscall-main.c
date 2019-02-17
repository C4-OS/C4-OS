#include <linux-compat/syscalls.h>
#include <c4rt/c4rt.h>

static compat_syscall_handler syscall_table[LINUX_COMPAT_MAX_SYSCALLS] = {
	// TODO
};

static const char *syscall_names[LINUX_COMPAT_MAX_SYSCALLS] = {
	// TODO
};

#define DEBUGF(FMT, ...) (c4_debug_printf("--- linux-compat: " FMT, __VA_ARGS__))

long linux_compat_syscall(long n, long a1, long a2, long a3,
                                  long a4, long a5, long a6)
{
	DEBUGF("syscall %u(%x, %x, %x, %x, %x, %x)", n, a1, a2, a3, a4, a5, a6);

	if (n >= 0 && n < LINUX_COMPAT_MAX_SYSCALLS && syscall_table[n]) {
		return syscall_table[n](a1, a2, a3, a4, a5, a6);
	}

	return -1;
}
