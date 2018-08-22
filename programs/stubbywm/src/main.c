#include <c4rt/c4rt.h>
#include <c4rt/mem.h>
#include <c4rt/interface/peripheral.h>
#include <c4rt/interface/keyboard.h>
#include <c4rt/interface/mouse.h>
#include <c4rt/interface/framebuffer.h>
#include <c4rt/stublibc.h>

#include <stubbywm/stubbywm.h>
#include <stubbywm/mouse_icons.h>
#include <nameserver/nameserver.h>

#include <stdint.h>
#include <stdbool.h>

inline void draw_pixel(wm_t *state, int32_t x, int32_t y, uint32_t pixel) {
	if (x < 0 || y < 0){
		return;
	}

	if (x >= state->info.width || y >= state->info.height) {
		return;
	}

	// TODO: assumes 32 bpp, add support for others
	uint32_t *buf = state->buffer.vaddrptr;
	buf[(y * state->info.width) + x] = pixel;
}

inline void draw_rect(wm_t *state, stubby_rect_t *rect, uint32_t color) {
	for (unsigned y = 0; y < rect->height; y++) {
		for (unsigned x = 0; x < rect->width; x++) {
			draw_pixel(state, x + rect->coord.x, y + rect->coord.y, color);
		}
	}
}

static void draw_mouse(wm_t *state, uint32_t color) {
	/*
	stubby_rect_t r;

	r.coord = state->mouse;
	r.height = r.width = 16;

	draw_rect(state, &r, color);
	*/
	ppm_draw(&state->mouse_cursor, state, state->mouse);
}

static void draw_background(wm_t *state) {
	for (unsigned y = 0; y < state->info.height; y++) {
		for (unsigned x = 0; x < state->info.width; x++) {
			uint32_t pixel = 0x202000 | (x ^ y);
			draw_pixel(state, x, y, pixel);
		}
	}
}

// TODO: merge these into c4rt at some point, these are pretty useful
static void page_copy(void *dest, void *src) {
	uint32_t *a = dest;
	uint32_t *b = src;

	for (unsigned i = 0; i < PAGE_SIZE / 4; i++) {
		a[i] = b[i];
	}
}

static void pages_copy(void *dest, void *src, unsigned pages) {
	uint8_t *a = dest;
	uint8_t *b = src;

	for (unsigned i = 0; i < pages; i++) {
		page_copy(a, b);
		a += PAGE_SIZE;
		b += PAGE_SIZE;
	}
}

static void draw_framebuffer(wm_t *state) {
	unsigned size = (state->info.height * state->info.width * 4);
	unsigned pages = pager_size_to_pages(size);
	pages_copy(state->framebuffer, state->buffer.vaddrptr, pages);
}

static void handle_mouse(wm_t *state, mouse_event_t *ev) {
	state->mouse.x += ev->x;
	state->mouse.y -= ev->y;

	if (state->mouse.x < 0) state->mouse.x = 0;
	if (state->mouse.y < 0) state->mouse.y = 0;

	if (state->mouse.x >= state->info.width)
		state->mouse.x = state->info.width - 1;

	if (state->mouse.y >= state->info.height)
		state->mouse.y = state->info.height - 1;
}

static void handle_keyboard(wm_t *state, keyboard_event_t *ev) {
	// TODO
}

static void event_loop(wm_t *state) {
	message_t msg;
	keyboard_event_t kev = {};
	mouse_event_t mev = {};

	while (true) {
		draw_background(state);
		draw_mouse(state,
		           0x800080
		           | ((mev.flags & MOUSE_FLAG_LEFT)?   0xff0000 : 0)
		           | ((mev.flags & MOUSE_FLAG_RIGHT)?  0x0000ff : 0)
		           | ((mev.flags & MOUSE_FLAG_MIDDLE)? 0x00ff00 : 0)
						   );
		draw_framebuffer(state);
		c4rt_peripheral_wait_event(&msg, state->peripherals);

		switch (msg.type) {
			case MOUSE_MSG_EVENT:
				mouse_parse_event(&msg, &mev);
				handle_mouse(state, &mev);
				break;

			case KEYBOARD_MSG_EVENT:
				keyboard_parse_event(&msg, &kev);
				handle_keyboard(state, &kev);
				break;

			default:
				break;
		}
	}
}

int main(int argc, char *argv[]) {
	wm_t state;

	// initialize icons
	ppm_load(&state.mouse_cursor, cursor_default_ppm);

	// initialize framebuffer
	state.buf_server = nameserver_lookup(C4_NAMESERVER, "/dev/framebuffer");

	if (!state.buf_server) {
		c4_debug_printf("--- stubbywm: couldn't find framebuffer...\n");
		return 1;
	}

	framebuffer_get_info(state.buf_server, &state.info);
	c4_debug_printf("--- stubbywm: width: %u, height: %u\n",
	                state.info.width, state.info.height);

	state.buf_cap = framebuffer_get_buffer(state.buf_server);

	c4_debug_printf("--- stubbywm: cap: %u\n", state.buf_cap);
	c4_addrspace_map(C4_CURRENT_ADDRSPACE, state.buf_cap,
	                 0xfb000000, PAGE_READ | PAGE_WRITE);
	state.framebuffer = (void*)0xfb000000;
	memset(state.framebuffer, 0x80, state.info.width * state.info.height * 4);

	c4_memobj_alloc(&state.buffer,
	                state.info.width * state.info.height * 4,
	                PAGE_READ | PAGE_WRITE);
	c4_debug_printf("--- stubbywm: have buffer at %p\n", state.buffer.vaddrptr);

	// initialize inputs
	state.peripherals = c4_msg_create_async();
	state.mouse.x = state.mouse.y = 0;

	uint32_t keyboard = nameserver_lookup(C4_NAMESERVER, "/dev/keyboard");
	uint32_t mouse    = nameserver_lookup(C4_NAMESERVER, "/dev/mouse");

	if (!keyboard || !mouse) {
		c4_debug_printf("--- stubbywm: couldn't find peripherals...\n");
		return 1;
	}

	c4rt_peripheral_connect(keyboard, state.peripherals);
	c4rt_peripheral_connect(mouse, state.peripherals);

	event_loop(&state);

	return 0;
}
