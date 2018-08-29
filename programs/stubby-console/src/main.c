#include <c4rt/c4rt.h>
#include <c4rt/prng.h>
#include <c4rt/elf.h>
#include <c4rt/interface/framebuffer.h>
#include <c4rt/interface/peripheral.h>
#include <stubbywm/interface.h>
#include <nameserver/nameserver.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <stdlib.h>

typedef struct state {
	stubbywm_window_t window;

	c4_process_t nameserver;
	c4_process_t console;
	c4_process_t program;

	// XXX: only handle one keyboard client
	uint32_t keyboard;
} constate_t;

void start_nameserv(constate_t *state, const char *envp[]) {
	const char *nullargs[] = {NULL};

	state->nameserver = spawn("/sbin/nameserver", nullargs, envp);
	c4_cspace_move(C4_CURRENT_CSPACE, state->nameserver.endpoint,
	               C4_CURRENT_CSPACE, C4_NAMESERVER);
}

void start_servers(constate_t *state, const char *envp[]) {
	const char *nullargs[] = {NULL};
	const char *displayargs[] = {"/sbin/display", "--nested", NULL};

	state->console = spawn("/sbin/display", displayargs, envp);
	state->program = spawn("/bin/forth",    nullargs,    envp);
}

void draw_background(constate_t *state) {
	uint32_t *buf = state->window.buffer.vaddrptr;

	for (unsigned y = 0; y < state->window.height; y++) {
		for (unsigned x = 0; x < state->window.width; x++){
			unsigned index = (y * state->window.width) + x;
			buf[index] = 0xa0202020;
		}
	}
}

// TODO: once again, add a way to pass parameters to threads
static constate_t state;

void handle_keyboard_event(constate_t *state, message_t *msg) {
	if (!state->keyboard) {
		return;
	}

	message_t foo = *msg;
	foo.type = KEYBOARD_MSG_EVENT;
	c4_msg_send_async(&foo, state->keyboard);
}

void update_loop(void) {
	while (true) {
		uint32_t *buf = state.window.buffer.vaddrptr;
		message_t msg;
		c4_msg_recieve_async(&msg, state.window.from_port, MESSAGE_ASYNC_BLOCK);

		c4_debug_printf("--- stubby-console: got here\n");

		if (msg.type == STUBBYWM_KEYBOARD_EVENT) {
			handle_keyboard_event(&state, &msg);
		}

		stubbywm_update(&state.window);
	}
}

void handle_framebuf_request(constate_t *state, uint32_t port) {
	message_t msg;
	c4_msg_recieve(&msg, port);

	switch (msg.type) {
		case FRAMEBUFFER_MSG_INFO:
			msg = (message_t) {
				.type = FRAMEBUFFER_MSG_INFO,
				.data = {
					state->window.width,
					state->window.height,
					32,
				},
			};

			c4_msg_send(&msg, port);
			break;

		case FRAMEBUFFER_MSG_GET_BUFFER:
			c4_cspace_grant(state->window.buffer.page_obj, port,
			                CAP_ACCESS | CAP_MODIFY
			                | CAP_MULTI_USE | CAP_SHARE);

			break;

		default:
			break;
	}
}

void interface_loop(void) {
	while (true) {
		message_t msg;
		c4_debug_printf("--- stubby-console: interface loop waiting\n");
		c4_msg_recieve(&msg, C4_SERV_PORT);

		if (msg.type != MESSAGE_TYPE_GRANT_OBJECT) {
			continue;
		}

		int32_t temp = msg.data[5];

		switch (msg.data[0]) {
			case CAP_TYPE_IPC_ASYNC_ENDPOINT:
				// assume it's a peripheral connect
				state.keyboard = temp;
				c4_debug_printf("--- stubby-console: keyboard: %u\n", state.keyboard);
				break;

			case CAP_TYPE_IPC_SYNC_ENDPOINT:
			default:
				// otherwise assume a framebuffer info request
				handle_framebuf_request(&state, temp);
				c4_cspace_remove(C4_CURRENT_CSPACE, temp);
				break;
		}
	}
}

int main(int argc, const char *argv[], const char *envp[]){
	c4_debug_printf( "--- stubby-console: hello, world! thread %u\n",
	                 c4_get_id());
	memset(&state, 0, sizeof(constate_t));

	int32_t serv = nameserver_lookup(C4_NAMESERVER, "stubbywm");

	// TODO: add polling lookup call to nameserver interface
	for (unsigned i = 0; serv <= 0 && i < 100; i++) {
		serv = nameserver_lookup(C4_NAMESERVER, "stubbywm");
	}

	C4_ASSERT(serv > 0);

	state.window = stubbywm_new_window(serv, 480, 320);
	draw_background(&state);
	stubbywm_update(&state.window);

	start_nameserv(&state, envp);
	// bind interfaces that this provides
	nameserver_bind(C4_NAMESERVER, "/dev/framebuffer", C4_SERV_PORT);
	nameserver_bind(C4_NAMESERVER, "/dev/keyboard", C4_SERV_PORT);
	nameserver_bind(C4_NAMESERVER, "/dev/console-alert",
	                state.window.from_port);

	start_servers(&state, envp);

	static uint8_t worker_stack[PAGE_SIZE];
	// TODO: make a library function to handle all of this
	uint32_t tid = c4_create_thread(interface_loop, worker_stack + PAGE_SIZE, 0);
	c4_set_capspace(tid, C4_CURRENT_CSPACE);
	c4_set_addrspace(tid, C4_CURRENT_ADDRSPACE);
	c4_set_pager(tid, C4_PAGER);;
	c4_continue_thread(tid);

	update_loop();

	return 0;
}
