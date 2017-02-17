#ifndef _C4OS_DISPLAY_H
#define _C4OS_DISPLAY_H 1
#include <stdint.h>

enum {
	VGA_TEXT_WIDTH  = 80,
	VGA_TEXT_HEIGHT = 25,
	VGA_TEXT_START  = 0,
	VGA_TEXT_COLOR  = 0x17,
};

enum {
	GRAPHIC_FOREGROUND = 0xd0d0d0,
	GRAPHIC_BACKGROUND = 0x181818,
};

typedef struct vga_char {
	uint8_t text;
	uint8_t color;
} vga_char_t;

typedef struct psf2_header {
    uint8_t  magic[4];
    uint32_t version;
    uint32_t header_size;
    uint32_t flags;
    uint32_t length;
    uint32_t charsize;
    uint32_t height;
    uint32_t width;
} __attribute__((packed)) psf2_header_t;

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

void framebuffer_init( display_t *state );
void framebuf_clear( display_t *state );
void framebuf_scroll( display_t *state );
void framebuf_draw_char( display_t *state,
                         unsigned x,
                         unsigned y,
                         char c );

void textbuffer_init( display_t *state );
void text_scroll( display_t *state );
void text_clear( display_t *state );
void text_draw_char( display_t *state,
                     unsigned x,
                     unsigned y,
                     char c );
#endif
