#include <display/display.h>
#include <c4rt/c4rt.h>
#include <c4rt/interface/framebuffer.h>
#include <nameserver/nameserver.h>
#include <c4/bootinfo.h>
#include <c4/paging.h>

static bootinfo_t *c4_bootinfo = BOOTINFO_ADDR;

extern psf2_header_t display_font;
extern unsigned long display_font_size;

void framebuf_draw_char( display_t *state,
                         unsigned x,
                         unsigned y,
                         char c )
{
	uint8_t *bitmap = (uint8_t *)&display_font + display_font.header_size;
	unsigned mod = 1 << (display_font.width + 1);

	unsigned meh  = x * display_font.width;
	unsigned blah = y * state->fb_width * display_font.height;
	uint32_t *place = state->pixelbuf + blah + meh;

	for ( unsigned iy = 0; iy < display_font.height; iy++ ){
		for ( unsigned ix = 0; ix < display_font.width; ix++ ){
			if ( bitmap[c * display_font.charsize + iy] & (mod >> ix)){
				*place = GRAPHIC_FOREGROUND;

			} else {
				*place = GRAPHIC_BACKGROUND;
			}

			place += 1;
		}

		place += state->fb_width - display_font.width;
	}
}

void framebuf_clear( display_t *state ){

}

void framebuf_scroll( display_t *state ){
	unsigned rowsize = state->fb_width;
	unsigned rows    = state->fb_height - display_font.height - 1;

	for ( unsigned i = 0; i < rows; i++ ){
		unsigned offset = i * rowsize;
		unsigned next   = (i + display_font.height) * rowsize;

		for ( uint32_t k = 0; k < rowsize; k++ ){
			state->pixelbuf[k + offset] = state->pixelbuf[k + next];
		}
	}
}

void framebuffer_init_raw( display_t *state ){
	unsigned size =
		c4_bootinfo->framebuffer.width *
		c4_bootinfo->framebuffer.height *
		4;

	*state = (display_t){
		.pixelbuf   = (void *)0xfb000000,
		.fb_width   = c4_bootinfo->framebuffer.width,
		.fb_height  = c4_bootinfo->framebuffer.height,
		.width      = c4_bootinfo->framebuffer.width  / display_font.width,
		.height     = c4_bootinfo->framebuffer.height / display_font.height - 1,
		.x          = 0,
		.y          = 0,
		.notify_cap = 0,

		.draw_char  = framebuf_draw_char,
		.scroll     = framebuf_scroll,
		.clear      = framebuf_clear,
	};

	c4_debug_printf( "--- display: text buffer of %ux%u\n",
		state->width, state->height );

	state->buf_cap = c4_request_physical( 0xfb000000,
	                                     c4_bootinfo->framebuffer.addr,
	                                     size / PAGE_SIZE + 1,
	                                     PAGE_READ | PAGE_WRITE );

	uint32_t *fb = (void *)0xfb000000;

	for ( unsigned y = 0; y < c4_bootinfo->framebuffer.height; y++ ){
		for ( unsigned x = 0; x < c4_bootinfo->framebuffer.width; x++ ){
			unsigned index = y * c4_bootinfo->framebuffer.width + x;

			//fb[index] = 0x202000 | x ^ y;
			fb[index] = GRAPHIC_BACKGROUND;
		}
	}
}

void framebuffer_init_nested( display_t *state ){
	uint32_t serv = 0;

	for (unsigned i = 0; serv == 0 && i < 100; i++) {
		serv = nameserver_lookup(C4_NAMESERVER, "/dev/framebuffer");
	}

	framebuffer_info_t info;
	framebuffer_get_info(serv, &info);

	*state = (display_t){
		.fb_width  = info.width,
		.fb_height = info.height,
		.width     = info.width  / display_font.width,
		.height    = info.height / display_font.height - 1,
		.x         = 0,
		.y         = 0,

		.draw_char = framebuf_draw_char,
		.scroll    = framebuf_scroll,
		.clear     = framebuf_clear,
	};

	c4_debug_printf( "--- display: text buffer of %ux%u\n",
		state->width, state->height );

	state->buf_cap = framebuffer_get_buffer(serv);
	c4_debug_printf( "--- display: cap at %u\n",
		state->buf_cap);

	size_t size = info.width * info.height * 4;
	C4_ASSERT(c4_memobj_region_map(state->buf_cap, &state->buf, size,
	                               PAGE_READ | PAGE_WRITE));
	state->pixelbuf = state->buf.vaddrptr;
	state->notify_cap = nameserver_lookup(C4_NAMESERVER, "/dev/console-alert");

	uint32_t *fb = state->pixelbuf;

	for ( unsigned y = 0; y < info.height; y++ ){
		for ( unsigned x = 0; x < info.width; x++ ){
			unsigned index = y * info.width + x;
			fb[index] = GRAPHIC_BACKGROUND;
		}
	}
}
