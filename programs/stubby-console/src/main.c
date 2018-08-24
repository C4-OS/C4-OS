#include <c4rt/c4rt.h>
#include <c4rt/prng.h>
#include <stubbywm/interface.h>
#include <nameserver/nameserver.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>

int main(int argc, char *argv[]){
	c4_debug_printf( "--- stubby-console: hello, world! thread %u\n",
	                 c4_get_id());

	int32_t serv = nameserver_lookup(C4_NAMESERVER, "stubbywm");
	C4_ASSERT(serv > 0);

	stubbywm_window_t win = stubbywm_new_window(serv, 480, 320);

	memset(win.buffer.vaddrptr, 0xd0d0d0, win.width * win.height * 4);
	stubbywm_update(&win);

	while (true) {
		uint32_t *buf = win.buffer.vaddrptr;
		message_t msg;
		c4_msg_recieve_async(&msg, win.from_port, MESSAGE_ASYNC_BLOCK);

		c4_debug_printf("--- stubby-console: got here\n");

		for (unsigned y = 0; y < win.height; y++) {
			for (unsigned x = 0; x < win.width; x++){
				unsigned index = (y * win.width) + x;
				buf[index] = c4rt_prng_u32();
			}
		}

		stubbywm_update(&win);
	}

	return 0;
}
