#include <c4/paging.h>
#include <c4/bootinfo.h>
#include <c4/arch/interrupts.h>

#include <c4rt/c4rt.h>
#include <c4rt/interface/console.h>
#include <c4rt/interface/framebuffer.h>

#include <display/display.h>
#include <nameserver/nameserver.h>

#include <stdbool.h>
#include <stdint.h>

static inline void do_newline( display_t *state ){
	if ( ++state->y >= state->height - 1 ){
		state->scroll( state );
		state->y = state->height - 1;
	}

	state->x = 0;
}

static bootinfo_t *c4_bootinfo = BOOTINFO_ADDR;

static void do_draw_char( display_t *state, char c ){
	if ( c == '\n' ){
		do_newline( state );

	} else if ( c == '\b' ){
		state->x--;
		state->draw_char( state, state->x, state->y, ' ' );

	} else {

		state->draw_char( state, state->x, state->y, c );

		if ( state->x++ >= state->width ){
			do_newline( state );
		}
	}
}

static void do_set_position( display_t *state, unsigned x, unsigned y ){
	if ( x < state->width && y < state->height ){
		state->x = x;
		state->y = y;
	}

	// TODO: notify sender of an error
}

static void do_clear( display_t *state ){
	for ( unsigned i = 0; i < state->height; i++ ){
		do_newline( state );
	}

	do_set_position( state, 0, 0 );
}

static void send_info(message_t *msg, display_t *state, uint32_t port){
	message_t ret = {
		.type = CONSOLE_MSG_INFO,
		.data = {
			state->x,
			state->y,
			state->width,
			state->height,
		},
	};

	c4_msg_send(&ret, port);
}

static void send_framebuf_info(message_t *msg, display_t *state, uint32_t port){
	message_t ret = {
		.type = FRAMEBUFFER_MSG_INFO,
		.data = {
			c4_bootinfo->framebuffer.width,
			c4_bootinfo->framebuffer.height,
			32,
		},
	};

	c4_msg_send(&ret, port);
}

static void send_framebuffer(message_t *msg, display_t *state, uint32_t port){
	if (!c4_bootinfo->framebuffer.exists) {
		message_t msg = { .type = FRAMEBUFFER_MSG_ERROR, };
		c4_msg_send(&msg, port);
	}

	c4_cspace_grant(state->buf_cap, port,
	                CAP_ACCESS | CAP_MODIFY | CAP_MULTI_USE | CAP_SHARE);
}

void _start( uintptr_t nameserver ){
	message_t msg;
	display_t state;
	int serv_port = c4_msg_create_sync();

	if ( c4_bootinfo->framebuffer.exists ){
		framebuffer_init( &state );
		// TODO: consider splitting console and framebuffer programs, having both
		//       managed by the same program is convenient for now though
		nameserver_bind( nameserver, "/dev/framebuffer", serv_port );

	} else {
		textbuffer_init( &state );
	}

	nameserver_bind( nameserver, "/dev/console", serv_port );

	while ( true ){
		uint32_t temp = 0;
		c4_msg_recieve(&msg, serv_port);

		if (msg.type == MESSAGE_TYPE_GRANT_OBJECT) {
			temp = msg.data[5];
			C4_ASSERT(msg.data[0] == CAP_TYPE_IPC_SYNC_ENDPOINT);
			c4_msg_recieve(&msg, temp);
		}

		switch ( msg.type ){
			case CONSOLE_MSG_PUT_CHAR:
				do_draw_char(&state, msg.data[0]);
				break;

			case CONSOLE_MSG_SET_POSITION:
				do_set_position(&state, msg.data[0], msg.data[1]);
				break;

			case CONSOLE_MSG_CLEAR:
				do_clear( &state );
				break;

			case CONSOLE_MSG_GET_INFO:
				send_info(&msg, &state, temp);
				break;

			case FRAMEBUFFER_MSG_INFO:
				send_framebuf_info(&msg, &state, temp);
				break;

			case FRAMEBUFFER_MSG_GET_BUFFER:
				send_framebuffer(&msg, &state, temp);
				break;

			default:
				break;
		}

		if (temp) {
			c4_cspace_remove(C4_CURRENT_CSPACE, temp);
		}
	}
}
