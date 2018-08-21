#include <nameserver/nameserver.h>
#include <c4rt/c4rt.h>
#include <c4/arch/interrupts.h>
#include <c4rt/interface/mouse.h>

#include <stdbool.h>
#include <stdint.h>

enum {
	MOUSE_DATA_PORT   = 0x60,
	MOUSE_STATUS_PORT = 0x64,
};

enum {
	MOUSE_STATUS_CAN_READ  = (1 << 0),
	MOUSE_STATUS_CAN_WRITE = (1 << 1),
	MOUSE_STATUS_IS_MOUSE  = (1 << 5),
};

enum {
	MOUSE_COMMAND_COMPAQ_STATUS    = 0x20,
	MOUSE_COMMAND_SET_STATUS       = 0x60,
	MOUSE_COMMAND_SEND             = 0xd4,
	MOUSE_COMMAND_AUX_ENABLE       = 0xa8,
	MOUSE_COMMAND_GET_ID           = 0xf2,
	MOUSE_COMMAND_SET_SAMPLE_RATE  = 0xf3,
	MOUSE_COMMAND_ENABLE_STREAMING = 0xf4,
	MOUSE_COMMAND_SET_DEFAULTS     = 0xf6,
	MOUSE_COMMAND_RESET            = 0xff,
};

static bool wait_read(void) {
	unsigned timeout = 123456;

	while (timeout--) {
		if (c4_in_byte(0x64) & 1) {
			return true;
		}
	}

	c4_debug_printf("--- mouse: timeout on read\n");
	return false;
}

static bool wait_write(void) {
	unsigned timeout = 123456;

	while (timeout--) {
		if ((c4_in_byte(0x64) & 2) == 0) {
			return true;
		}
	}

	c4_debug_printf("--- mouse: timeout on write\n");
	return false;
}

static void mouse_write(uint8_t port, uint8_t thing) {
	wait_write();
	c4_out_byte(port, thing);
}

static uint8_t mouse_read(void) {
	wait_read();
	return c4_in_byte(MOUSE_DATA_PORT);
}

static uint8_t mouse_command(uint8_t command) {
	mouse_write(MOUSE_STATUS_PORT, MOUSE_COMMAND_SEND);
	mouse_write(MOUSE_DATA_PORT, command);
	return mouse_read();
}

static bool have_mouse = false;

void mouse_init(void) {
	mouse_write(MOUSE_STATUS_PORT, MOUSE_COMMAND_AUX_ENABLE);
	mouse_write(MOUSE_STATUS_PORT, MOUSE_COMMAND_COMPAQ_STATUS);
	wait_read();

	uint8_t status = c4_in_byte(MOUSE_DATA_PORT);
	status |= 2;
	status &= ~(1 << 5);
	mouse_write(MOUSE_STATUS_PORT, MOUSE_COMMAND_SET_STATUS);
	mouse_write(MOUSE_DATA_PORT, status);
	c4_debug_printf("--- mouse: initialized stuff: %x\n", status);

	mouse_command(MOUSE_COMMAND_RESET);
	mouse_command(MOUSE_COMMAND_SET_DEFAULTS);
	mouse_command(MOUSE_COMMAND_ENABLE_STREAMING);

	mouse_command(MOUSE_COMMAND_SET_SAMPLE_RATE);
	mouse_write(MOUSE_DATA_PORT, 40);

	uint8_t id = 0xfa;
	id = mouse_command(MOUSE_COMMAND_GET_ID);

	for (unsigned k = 0; id > 4 && k < 3; k++) {
		if (!wait_read()) {
			continue;
		}

		id = mouse_read();
	}

	C4_ASSERT(id <= 4);
	c4_debug_printf("--- mouse: ID: %u\n", id);
	c4_debug_printf("--- mouse: sent things\n");
}

static int32_t interrupt_queue = -1;
static uint32_t clients[1024];
static unsigned num_clients;

void dispatch(uint8_t *packet) {
	mouse_event_t ev;

	if ((packet[0] & 0x40)|| (packet[0] & 0x80)) {
		return;
	}

	ev.flags = packet[0] & 0x7;
	ev.x = (packet[0] & 0x10)? ~packet[1] : packet[1];
	ev.y = (packet[0] & 0x20)? ~packet[2] : packet[2];

	message_t msg = {
		.type = MOUSE_MSG_EVENT,
		.data = {
			ev.flags,
			ev.x, ev.y,
		},
	};

	for (unsigned k = 0; k < num_clients; k++) {
		c4_msg_send_async(&msg, clients[k]);
	}
}

static void event_thread(void) {
	uint8_t things[8];
	unsigned counter = 0;

	while (true) {
		message_t msg;

		c4_msg_recieve_async(&msg, interrupt_queue, MESSAGE_ASYNC_BLOCK);
		wait_read();

		if (!(c4_in_byte(MOUSE_STATUS_PORT) & MOUSE_STATUS_IS_MOUSE)) {
			continue;
		}

		things[counter++] = c4_in_byte(0x60);

		if (counter == 3) {
			dispatch(things);
			counter = 0;
		}
	}
}

int main(int argc, char *argv[]) {
	nameserver_bind(C4_NAMESERVER, "/dev/mouse", C4_SERV_PORT);

	num_clients = 0;
	interrupt_queue = c4_msg_create_async();

	mouse_init();
	c4_interrupt_subscribe(0x20 + 12, interrupt_queue);

	static uint8_t worker_stack[PAGE_SIZE];

	// TODO: make a library function to handle all of this
	uint32_t tid = c4_create_thread(event_thread, worker_stack + PAGE_SIZE, 0);
	c4_set_capspace(tid, C4_CURRENT_CSPACE);
	c4_set_addrspace(tid, C4_CURRENT_ADDRSPACE);
	c4_set_pager(tid, C4_PAGER);;
	c4_continue_thread(tid);

	while (true) {
		message_t msg;
		c4_msg_recieve(&msg, C4_SERV_PORT);

		if (msg.type != MESSAGE_TYPE_GRANT_OBJECT) {
			continue;
		}

		C4_ASSERT(msg.data[0] == CAP_TYPE_IPC_ASYNC_ENDPOINT);
		// TODO: locking
		clients[num_clients++] = msg.data[5];
		c4_debug_printf("--- mouse: now have %u clients\n", num_clients);
	}

	return 0;
}
