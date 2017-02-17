#include <display/display.h>
#include <c4rt/c4rt.h>
#include <c4/paging.h>

void text_draw_char( display_t *state,
                     unsigned x,
                     unsigned y,
                     char c )
{
	vga_char_t *temp = state->textbuf + state->width * state->y + state->x;

	temp->text = c;
	temp->color = VGA_TEXT_COLOR;
}

void text_scroll( display_t *state ){
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

void text_clear( display_t *state ){
	state->x = 0;
	state->y = 0;

	for ( unsigned i = 0; i < state->width * state->height; i++ ){
		state->textbuf[i].text  = ' ';
		state->textbuf[i].color = VGA_TEXT_COLOR;
	}
}

void textbuffer_init( display_t *state ){
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
