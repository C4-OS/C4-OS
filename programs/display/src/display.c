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

enum {
	VGA_TEXT_WIDTH  = 80,
	VGA_TEXT_HEIGHT = 25,
	VGA_TEXT_START  = 0,
	VGA_TEXT_COLOR  = 0x17,
};

typedef struct vga_char {
	uint8_t text;
	uint8_t color;
} vga_char_t;

typedef struct display {
	union {
		vga_char_t *textbuf;
		uint32_t   *pixelbuf;
	};

	void (*draw_char)( struct display *state, unsigned x, unsigned y, char c );
	void (*clear)( struct display *state );
	void (*scroll)( struct display *state );

	unsigned x;
	unsigned y;

	// width and height are in characters, for the framebuffer
	// this will be calculated by dividing screen dimensions by
	// font dimensions
	unsigned width;
	unsigned height;
} display_t;

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
static void framebuffer_init( display_t *state );
static void textbuffer_init( display_t *state );

static inline void framebuf_draw_char( display_t *state,
                                       unsigned x,
                                       unsigned y,
                                       char c )
{
	c4_debug_putchar( c );
}

static inline void framebuf_clear( display_t *state ){

}

static inline void framebuf_scroll( display_t *state ){

}

static void framebuffer_init( display_t *state ){
	unsigned size =
		c4_bootinfo->framebuffer.width *
		c4_bootinfo->framebuffer.height *
		4;

	*state = (display_t){
		.pixelbuf = (void *)0xfb000000,
		.x        = 0,
		.y        = 0,
		.width    = 80,
		.height   = 25,

		.draw_char = framebuf_draw_char,
		.scroll    = framebuf_scroll,
		.clear     = framebuf_clear,
	};

	c4_request_physical( 0xfb000000,
	                     c4_bootinfo->framebuffer.addr,
	                     size / PAGE_SIZE + 1,
	                     PAGE_READ | PAGE_WRITE );

	uint32_t *fb = (void *)0xfb000000;

	for ( unsigned y = 0; y < c4_bootinfo->framebuffer.height; y++ ){
		for ( unsigned x = 0; x < c4_bootinfo->framebuffer.width; x++ ){
			unsigned index = y * c4_bootinfo->framebuffer.width + x;

			fb[index] = ((y & 0xff) << 16) | ((x & 0xff) << 8) | (x ^ y);
		}
	}
}

static inline void text_draw_char( display_t *state,
                                   unsigned x,
                                   unsigned y,
                                   char c )
{
	vga_char_t *temp = state->textbuf + state->width * state->y + state->x;

	temp->text = c;
	temp->color = VGA_TEXT_COLOR;
}

static inline void text_scroll( display_t *state ){
	for ( unsigned i = 1; i < state->height; i++ ){
		vga_char_t *foo = state->textbuf + state->width * (i - 1);
		vga_char_t *bar = state->textbuf + state->width * i;

		for ( unsigned k = 0; k < state->width; k++ ){
			*foo++ = *bar++;
		}

		for ( unsigned k = 0; k < state->width; k++ ){
			foo->text = ' ';
			foo++;
		}
	}
}

static void text_clear( display_t *state ){
	state->x = 0;
	state->y = 0;

	for ( unsigned i = 0; i < state->width * state->height; i++ ){
		state->textbuf[i].text  = ' ';
		state->textbuf[i].color = VGA_TEXT_COLOR;
	}
}

static void textbuffer_init( display_t *state ){
	*state = (display_t){
		.textbuf = (void *)0xb8000,
		.x       = 0,
		.y       = 0,
		.width   = VGA_TEXT_WIDTH,
		.height  = VGA_TEXT_HEIGHT,

		.draw_char = text_draw_char,
		.clear     = text_clear,
		.scroll    = text_scroll,
	};

	// request access to the vga text buffer
	c4_request_physical( 0xb8000, 0xb8000, 1, PAGE_READ | PAGE_WRITE );

	for ( unsigned i = 0; i < VGA_TEXT_WIDTH * VGA_TEXT_HEIGHT; i++ ){
		state->textbuf[i].text = ' ';
		state->textbuf[i].color = VGA_TEXT_COLOR;
	}

}

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
