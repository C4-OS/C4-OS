#include <c4rt/c4rt.h>
#include <c4/paging.h>
#include <c4/arch/interrupts.h>
#include <stdbool.h>
#include <stdint.h>

enum {
	NAME_BIND = 0x1024,
	NAME_UNBIND,
	NAME_LOOKUP,
	NAME_RESULT,
};

enum {
	WIDTH  = 80,
	START  = 0,
	HEIGHT = 25,
};

typedef struct vga_char {
	uint8_t text;
	uint8_t color;
} vga_char_t;

typedef struct vga_state {
	vga_char_t *textbuf;
	unsigned x;
	unsigned y;
} vga_state_t;

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

static inline void scroll_display( vga_state_t *state ){
	for ( unsigned i = START + 1; i < HEIGHT; i++ ){
		vga_char_t *foo = state->textbuf + WIDTH * (i - 1);
		vga_char_t *bar = state->textbuf + WIDTH * i;

		for ( unsigned k = 0; k < WIDTH; k++ ){
			*foo++ = *bar++;
		}

		for ( unsigned k = 0; k < WIDTH; k++ ){
			foo->text = ' ';
			foo++;
		}
	}
}

static inline void do_newline( vga_state_t *state ){
	if ( ++state->y >= HEIGHT - 1 ){
		scroll_display( state );
		state->y = HEIGHT - 1;
	}

	state->x = 0;
}

void _start( uintptr_t nameserver ){
	static const uint8_t color = 0x17;
	message_t msg;

	vga_state_t state = {
		.textbuf = (void *)0xb8000,
		.x       = 0,
		.y       = START,
	};

	nameserver_bind( nameserver, "/dev/console" );

	// request access to the vga text buffer
	c4_request_physical( 0xb8000, 0xb8000, 1, PAGE_READ | PAGE_WRITE );

	for ( unsigned i = 0; i < WIDTH * HEIGHT; i++ ){
		state.textbuf[i].text = ' ';
		state.textbuf[i].color = color;
	}

	while ( true ){
		c4_msg_recieve( &msg, 0 );

		char c = msg.data[0];

		if ( c == '\n' ){
			do_newline( &state );

		} else if ( c == '\b' ){
			state.x--;

			vga_char_t *temp = state.textbuf + WIDTH * state.y + state.x;
			temp->text = ' ';

		} else {
			vga_char_t *temp = state.textbuf + WIDTH * state.y + state.x;

			temp->text  = c;
			temp->color = color;

			if ( state.x++ >= WIDTH ){
				do_newline( &state );
			}
		}
	}
}
