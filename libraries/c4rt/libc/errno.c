#include <errno.h>

int errno;

char *strerror(int errnum) {
	return "TODO: implement strerror()";
}

void perror(const char *s) {
	c4_debug_printf("TODO: implement perror()\n");
}
