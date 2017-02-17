#include <display/display.h>
#include <c4rt/c4rt.h>
#include <c4/paging.h>
#include <c4/bootinfo.h>
#include <c4/arch/interrupts.h>
#include <stdbool.h>
#include <stdint.h>

enum {
	NAME_BIND = 0x1024,
	NAME_UNBIND,
	NAME_LOOKUP,
	NAME_RESULT,
};

unsigned hash_string( const char *str ){
	unsigned hash = 757;
	int c;

	while (( c = *str++ )){
		hash = ((hash << 7) + hash + c);
	}

	return hash;
}

static inline void nameserver_bind( unsigned server, const char *name ){
	message_t msg = {
		.type = NAME_BIND,
		.data = { hash_string(name) },
	};

	c4_msg_send( &msg, server );
}

static inline unsigned nameserver_lookup( unsigned server, const char *name ){
	message_t msg = {
		.type = NAME_LOOKUP,
		.data = { hash_string(name) },
	};

	c4_msg_send( &msg, server );
	c4_msg_recieve( &msg, server );

	return msg.data[0];
}

static inline void do_newline( display_t *state ){
	if ( ++state->y >= state->height - 1 ){
		state->scroll( state );
		state->y = state->height - 1;
	}

	state->x = 0;
}

static bootinfo_t *c4_bootinfo = BOOTINFO_ADDR;

void _start( uintptr_t nameserver ){
	message_t msg;
	display_t state;

	if ( c4_bootinfo->framebuffer.exists ){
		framebuffer_init( &state );

	} else {
		textbuffer_init( &state );
	}

	nameserver_bind( nameserver, "/dev/console" );

	while ( true ){
		c4_msg_recieve( &msg, 0 );

		char c = msg.data[0];

		if ( c == '\n' ){
			do_newline( &state );

		} else if ( c == '\b' ){
			state.x--;
			state.draw_char( &state, state.x, state.y, ' ' );

		} else {

			state.draw_char( &state, state.x, state.y, c );

			if ( state.x++ >= state.width ){
				do_newline( &state );
			}
		}
	}
}
