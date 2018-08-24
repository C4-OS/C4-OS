#ifndef _C4OS_STUBBY_INTERFACE_H
#define _C4OS_STUBBY_INTERFACE_H 1

#include <c4rt/c4rt.h>
#include <c4rt/addrman.h>
#include <c4rt/mem.h>
#include <stdint.h>

typedef struct stubby_window {
	uint32_t width;
	uint32_t height;
	c4_mem_object_t buffer;
	uint32_t id;

	uint32_t to_port;
	uint32_t from_port;
	uint32_t sync_port;

	uint32_t update_port;
} stubbywm_window_t;

enum {
	STUBBYWM_NEW_WINDOW = 0xdabad00,
	STUBBYWM_NEW_WINDOW_RESPONSE,
	STUBBYWM_CLOSE_WINDOW,
	STUBBYWM_CLOSE_WINDOW_RESPONSE,
	STUBBYWM_PING,
	STUBBYWM_PONG,
	STUBBYWM_UPDATE,

	STUBBYWM_KEYBOARD_EVENT,
	STUBBYWM_MOUSE_EVENT,
	STUBBYWM_WINDOW_EVENT,
};

static inline int32_t recv_obj(uint32_t port, unsigned type) {
	message_t msg;
	c4_msg_recieve(&msg, port);

	C4_ASSERT(msg.type == MESSAGE_TYPE_GRANT_OBJECT);
	C4_ASSERT(msg.data[0] == type);

	return msg.data[5];
}

stubbywm_window_t stubbywm_new_window(uint32_t server,
                                      unsigned width,
                                      unsigned height)
{
	stubbywm_window_t ret;
	message_t msg = {
		.type = STUBBYWM_NEW_WINDOW,
		.data = {
			0, 0,
			width,
			height,
		},
	};

	int32_t temp = c4_send_temp_endpoint(server);
	c4_msg_send(&msg, temp);
	c4_msg_recieve(&msg, temp);
	C4_ASSERT(msg.type == STUBBYWM_NEW_WINDOW_RESPONSE);

	ret.width  = msg.data[2];
	ret.height = msg.data[3];
	ret.id     = msg.data[4];

	ret.from_port   = recv_obj(temp, CAP_TYPE_IPC_ASYNC_ENDPOINT);
	ret.to_port     = recv_obj(temp, CAP_TYPE_IPC_ASYNC_ENDPOINT);
	ret.update_port = recv_obj(temp, CAP_TYPE_IPC_ASYNC_ENDPOINT);
	ret.sync_port   = recv_obj(temp, CAP_TYPE_IPC_SYNC_ENDPOINT);

	int32_t page = recv_obj(temp, CAP_TYPE_PHYS_MEMORY);
	// TODO: add phys_frame_info() syscall to get size and permissions
	size_t  size = ret.width * ret.height * 4;

	c4_memobj_region_map(page, &ret.buffer, size, PAGE_READ | PAGE_WRITE);

	return ret;
}

void stubbywm_update(stubbywm_window_t *win) {
	message_t msg = {
		.type = STUBBYWM_UPDATE,
		.data = { win->id },
	};

	c4_msg_send_async(&msg, win->update_port);
}

#endif
