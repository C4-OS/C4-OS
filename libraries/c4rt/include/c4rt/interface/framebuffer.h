#ifndef _C4OS_FRAMEBUFFER_INTERFACE_H
#define _C4OS_FRAMEBUFFER_INTERFACE_H 1
#include <c4rt/c4rt.h>
#include <c4/message.h>

enum {
	FRAMEBUFFER_MSG_INFO = 0xcabba9e5,
	FRAMEBUFFER_MSG_GET_BUFFER,
	FRAMEBUFFER_MSG_ERROR,
};

typedef struct framebuffer_info {
	unsigned width;
	unsigned height;
	unsigned bpp;
} framebuffer_info_t;

static inline void framebuffer_get_info(uint32_t server, framebuffer_info_t *info) {
	int temp = c4_send_temp_endpoint(server);

	message_t msg = {
		.type = FRAMEBUFFER_MSG_INFO,
	};

	c4_msg_send(&msg, temp);
	c4_msg_recieve(&msg, temp);

	info->width  = msg.data[0];
	info->height = msg.data[1];
	info->bpp    = msg.data[2];

	c4_cspace_remove(C4_CURRENT_CSPACE, temp);
}

static inline uint32_t framebuffer_get_buffer(uint32_t server) {
	int temp = c4_send_temp_endpoint(server);

	message_t msg = {
		.type = FRAMEBUFFER_MSG_GET_BUFFER,
	};

	c4_msg_send(&msg, temp);
	c4_msg_recieve(&msg, temp);

	C4_ASSERT(msg.type == MESSAGE_TYPE_GRANT_OBJECT);
	c4_cspace_remove(C4_CURRENT_CSPACE, temp);

	return msg.data[5];
}

#endif
