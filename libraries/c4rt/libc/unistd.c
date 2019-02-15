#include <unistd.h>
#include <c4rt/c4rt.h>

int usleep(useconds_t useconds) {
	c4_sleep(useconds);
	return 0;
}

unsigned int sleep(unsigned int seconds) {
	c4_sleep(seconds * 1000000);
	return 0;
}
