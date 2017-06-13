#ifndef _C4OS_CONSOLE_INTERFACE_H
#define _C4OS_CONSOLE_INTERFACE_H 1
#include <c4rt/c4rt.h>
#include <c4/message.h>

enum {
	CONSOLE_MSG_PING = MESSAGE_TYPE_END_RESERVED,
	CONSOLE_MSG_PONG,
	CONSOLE_MSG_CLEAR,
	CONSOLE_MSG_PUT_CHAR,
	CONSOLE_MSG_SET_POSITION,
	CONSOLE_MSG_SET_COLOR,

	CONSOLE_MSG_GET_INFO,

	CONSOLE_MSG_INFO,
	CONSOLE_MSG_ERROR,
	CONSOLE_MSG_END,
};

typedef struct console_info {
	unsigned x_pos;
	unsigned y_pos;
	unsigned width;
	unsigned height;
} console_info_t;

static inline void console_clear( unsigned id ){
	message_t msg = { .type = CONSOLE_MSG_CLEAR, };

	c4_msg_send( &msg, id );
}

static inline void console_put_char( unsigned id, unsigned c ){
	message_t msg = {
		.type = CONSOLE_MSG_PUT_CHAR,
		.data = { c },
	};

	c4_msg_send( &msg, id );
}

static inline void console_set_position( unsigned id, unsigned x, unsigned y ){
	message_t msg = {
		.type = CONSOLE_MSG_SET_POSITION,
		.data = { x, y },
	};

	c4_msg_send( &msg, id );
}

static inline void console_set_color( unsigned id, unsigned color ){
	message_t msg = {
		.type = CONSOLE_MSG_SET_COLOR,
		.data = { color },
	};

	c4_msg_send( &msg, id );
}

static inline void console_get_info( unsigned id, console_info_t *info ){
	message_t msg = { .type = CONSOLE_MSG_GET_INFO, };

	c4_msg_send( &msg, id );
	c4_msg_recieve( &msg, id );

	info->x_pos  = msg.data[0];
	info->y_pos  = msg.data[1];
	info->width  = msg.data[2];
	info->height = msg.data[3];
}

#endif
