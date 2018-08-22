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

static void reset_updates(wm_t *wm) {
	wm->num_updates = 0;
}

static void normalize_point(wm_t *wm, stubby_point_t *pt) {
	// sanity checking
	if (pt->x < 0) pt->x = 0;
	if (pt->y < 0) pt->y = 0;

	if (pt->x > wm->info.width)
		pt->x = wm->info.width;

	if (pt->y > wm->info.height)
		pt->y = wm->info.height;
}

static wm_update_t *alloc_update(wm_t *wm) {
	wm_update_t *ret = wm->updates + wm->num_updates;
	wm->num_updates++;

	// TODO: just do a full update if the maximum number of updates is reached
	C4_ASSERT(wm->num_updates < MAX_UPDATES);
	wm->num_updates %= MAX_UPDATES;

	return ret;
}

static void set_full_update(wm_t *wm) {
	wm_update_t *up = alloc_update(wm);

	up->lower.x = 0;
	up->lower.y = 0;

	up->upper.x = wm->info.width;
	up->upper.y = wm->info.height;
}

static bool point_less_than(stubby_point_t *a, stubby_point_t *b) {
	return a->y < b->y || (a->y == b->y && a->x < b->x);
}

static void update_region(wm_t *wm, stubby_point_t *a, stubby_point_t *b) {
	wm_update_t *up = alloc_update(wm);

	up->lower = point_less_than(a, b)? *a : *b;
	up->upper = point_less_than(a, b)? *b : *a;

	// sanity checking
	normalize_point(wm, &up->lower);
	normalize_point(wm, &up->upper);
}

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
	ppm_draw(&state->mouse_cursor, state, state->mouse);
}

static void draw_background(wm_t *state) {
	for (unsigned k = 0; k < state->num_updates; k++) {
		wm_update_t up = state->updates[k];

		for (unsigned y = up.lower.y; y < up.upper.y; y++) {
			for (unsigned x = up.lower.x; x < up.upper.x; x++) {
				uint32_t pixel = 0x202000 | (x ^ y);
				draw_pixel(state, x, y, pixel);
			}
		}
	}
}

static stubby_point_t rect_max_coord(wm_t *wm, stubby_rect_t *rect){
	stubby_point_t temp = {
		.x = rect->coord.x + rect->width,
		.y = rect->coord.y + rect->height,
	};

	normalize_point(wm, &temp);
	return temp;
}

static void draw_window(wm_t *state, window_t *window) {
	for (unsigned k = 0; k < state->num_updates; k++) {
		wm_update_t up = state->updates[k];

		stubby_point_t a = window->rect.coord;
		stubby_point_t b = rect_max_coord(state, &window->rect);

		if (a.y >= up.upper.y || b.y < up.lower.y) {
			continue;
		}

		if (a.x >= up.upper.x || b.x < up.lower.x) {
			continue;
		}

		unsigned y_start = (a.y > up.lower.y)? a.y : up.lower.y;
		unsigned x_start = (a.x > up.lower.x)? a.x : up.lower.x;

		for (unsigned y = y_start; y < b.y && y < up.upper.y; y++) {
			for (unsigned x = x_start; x < b.x && x < up.upper.x; x++) {
				draw_pixel(state, x, y, window->color);
			}
		}
	}
}

static void draw_windows(wm_t *state) {
	window_node_t *node = state->winlist.start;

	for (; node; node = node->next) {
		draw_window(state, &node->window);
	}
}

// TODO: merge these into c4rt at some point, these are pretty useful
/*
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
*/

static void draw_framebuffer(wm_t *state) {
	/*
	   unsigned size = (state->info.height * state->info.width * 4);
	   unsigned pages = pager_size_to_pages(size);
	   pages_copy(state->framebuffer, state->buffer.vaddrptr, pages);
	   */

	if (state->num_updates == 0) {
		return;
	}

	uint32_t *fbuf = (void*)state->framebuffer;
	uint32_t *dbuf = (void*)state->buffer.vaddrptr;

	for (unsigned k = 0; k < state->num_updates; k++) {
		wm_update_t up = state->updates[k];

		for (unsigned y = up.lower.y; y < up.upper.y; y++){
			for (unsigned x = up.lower.x; x < up.upper.x; x++) {
				unsigned index = (y * state->info.width) + x;
				fbuf[index] = dbuf[index];
			}
		}
	}
}

static void update_mouse_region(wm_t *wm) {
	stubby_point_t offset;

	offset = wm->mouse;
	offset.x += wm->mouse_cursor.width;
	offset.y += wm->mouse_cursor.height;

	update_region(wm, &wm->mouse, &offset);
}

static void update_window_region(wm_t *wm, window_t *window) {
	stubby_point_t a = window->rect.coord;
	stubby_point_t b = rect_max_coord(wm, &window->rect);

	update_region(wm, &a, &b);
}

static void handle_mouse(wm_t *wm, mouse_event_t *ev) {
	update_mouse_region(wm);

	window_node_t *node = wm->winlist.end;

	for (; node; node = node->prev) {
		stubby_point_t a = node->window.rect.coord;
		stubby_point_t b = rect_max_coord(wm, &node->window.rect);

		if (wm->mouse.x >= a.x && wm->mouse.y >= a.y
		 && wm->mouse.x <  b.x && wm->mouse.y <  b.y)
		{

			if (ev->flags & MOUSE_FLAG_LEFT) {
				window_t win = node->window;

				update_window_region(wm, &win);
				window_list_remove(&wm->winlist, node);
				window_list_insert(&wm->winlist, &win);
				update_window_region(wm, &win);
				break;
			}

			else if (ev->flags & MOUSE_FLAG_RIGHT) {
				update_window_region(wm, &node->window);

				// TODO: move this into window_set_pos()
				stubby_point_t pt = node->window.rect.coord;
				pt.x += ev->x;
				pt.y -= ev->y;
				normalize_point(wm, &pt);
				window_set_pos(&node->window, pt.x, pt.y);

				update_window_region(wm, &node->window);
				break;
			}
		}
	}

	wm->mouse.x += ev->x;
	wm->mouse.y -= ev->y;

	normalize_point(wm, &wm->mouse);
	update_mouse_region(wm);
}

static void handle_keyboard(wm_t *state, keyboard_event_t *ev) {
	// TODO
}

static void event_loop(wm_t *state) {
	message_t msg;
	keyboard_event_t kev = {};
	mouse_event_t mev = {};

	reset_updates(state);
	set_full_update(state);

	while (true) {
		draw_background(state);
		draw_windows(state);
		draw_mouse(state,
		           0x800080
		           | ((mev.flags & MOUSE_FLAG_LEFT)?   0xff0000 : 0)
		           | ((mev.flags & MOUSE_FLAG_RIGHT)?  0x0000ff : 0)
		           | ((mev.flags & MOUSE_FLAG_MIDDLE)? 0x00ff00 : 0));
		draw_framebuffer(state);
		reset_updates(state);

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
	memset(&state, 0, sizeof(state));

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

	// create some dummy windows
	window_t w = window_create(256, 256);
	window_set_pos(&w, 128, 128);
	window_list_insert(&state.winlist, &w);

	w = window_create(192, 192);
	window_set_pos(&w, 256, 256);
	window_list_insert(&state.winlist, &w);

	w = window_create(64, 256);
	window_set_pos(&w, 384, 384);
	window_list_insert(&state.winlist, &w);

	w = window_create(256, 128);
	window_set_pos(&w, 512, 512);
	window_list_insert(&state.winlist, &w);

	event_loop(&state);

	return 0;
}
