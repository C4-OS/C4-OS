#include <c4rt/c4rt.h>
#include <c4rt/mem.h>
#include <c4rt/interface/peripheral.h>
#include <c4rt/interface/keyboard.h>
#include <c4rt/interface/mouse.h>
#include <c4rt/interface/framebuffer.h>
#include <c4rt/connman.h>

#include <stubbywm/stubbywm.h>
#include <stubbywm/mouse_icons.h>
#include <stubbywm/interface.h>
#include <nameserver/nameserver.h>

#include <string.h>
#include <stdint.h>
#include <stdbool.h>
#include <unistd.h>

#define STUBBYWM_ENABLE_TRANSPARENCY 1

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

static void set_full_update(wm_t *wm);
static wm_update_t *alloc_update(wm_t *wm) {
	wm_update_t *ret = wm->updates + wm->num_updates;
	wm->num_updates++;

	if (wm->num_updates >= MAX_UPDATES) {
		reset_updates(wm);
		set_full_update(wm);
		c4_debug_printf("--- stubbywm: did full update!\n");
	}

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

static inline uint8_t mix_channel(uint8_t a, uint8_t b, uint8_t alpha) {
	uint32_t lower = a * (255 - alpha);
	uint32_t upper = b * alpha;

	return (lower + upper) / 255;
}

inline void draw_pixel(wm_t *state, int32_t x, int32_t y, uint32_t pixel) {
	if (x < 0 || y < 0){
		return;
	}

	if (x >= state->info.width || y >= state->info.height) {
		return;
	}

#ifdef STUBBYWM_ENABLE_TRANSPARENCY
	if ((pixel & 0xff000000) != 0) {
		uint32_t *buf = state->buffer.vaddrptr;
		uint32_t temp = buf[(y * state->info.width) + x];
		uint8_t alpha = (pixel >> 24);

		uint8_t t_red   = temp >> 16;
		uint8_t t_green = temp >> 8;
		uint8_t t_blue  = temp;

		uint8_t p_red   = pixel >> 16;
		uint8_t p_green = pixel >> 8;
		uint8_t p_blue  = pixel;

		uint32_t comp = (mix_channel(t_red, p_red, alpha) << 16)
		                | (mix_channel(t_green, p_green, alpha) << 8)
		                | (mix_channel(t_blue, p_blue, alpha));

		buf[(y * state->info.width) + x] = comp;
		return;
	}
#endif

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

static void draw_mouse(wm_t *state) {
	ppm_draw(&state->mouse_cursor, state, state->mouse);
}

static void draw_background(wm_t *state) {
	for (unsigned k = 0; k < state->num_updates; k++) {
		wm_update_t up = state->updates[k];

		for (unsigned y = up.lower.y; y < up.upper.y; y++) {
			for (unsigned x = up.lower.x; x < up.upper.x; x++) {
				uint8_t thing = ((x ^ y) >> 2) + 0xa0;
				uint32_t pixel = ((thing >> 1) << 16) | (thing << 9) | thing;
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
				if (window->has_buffer) {
					uint32_t *buf = window->buffer.vaddrptr;
					int32_t yoff = ((y - a.y) * window->rect.width);
					int32_t xoff = x - a.x;
					int32_t index = yoff + xoff;

					draw_pixel(state, x, y, buf[index]);
				}
				else {
					draw_pixel(state, x, y, window->color);
				}
			}
		}
	}
}

static void draw_window_decoration(wm_t *state, window_t *window) {
	// TODO: make configurable
	uint32_t color = 0x202020;
	unsigned border_size = 3;

	stubby_rect_t titlebar = (stubby_rect_t){
		.coord = (stubby_point_t) {
			.x = window->rect.coord.x - border_size,
			.y = window->rect.coord.y - 25,
		},
		.height = 25,
		.width = window->rect.width + (border_size * 2),
	};

	stubby_rect_t text_placeholder = (stubby_rect_t){
		.coord = (stubby_point_t) {
			.x = window->rect.coord.x + 5 - border_size,
			.y = window->rect.coord.y - 20,
		},
		.height = 15,
		.width = window->rect.width / 4,
	};

	stubby_rect_t left_border = (stubby_rect_t){
		.coord = (stubby_point_t) {
			.x = window->rect.coord.x - border_size,
			.y = window->rect.coord.y,
		},
		.height = window->rect.height + border_size,
		.width = border_size,
	};

	stubby_rect_t right_border = (stubby_rect_t){
		.coord = (stubby_point_t) {
			.x = window->rect.coord.x + window->rect.width,
			.y = window->rect.coord.y,
		},
		.height = window->rect.height + border_size,
		.width = border_size,
	};

	stubby_rect_t bottom_border = (stubby_rect_t){
		.coord = (stubby_point_t) {
			.x = window->rect.coord.x,
			.y = window->rect.coord.y + window->rect.height,
		},
		.height = border_size,
		.width = window->rect.width,
	};

	// XXX: redraw full border and title bar, even if they're not touched
	// TODO: more efficient
	draw_rect(state, &titlebar, color);
	draw_rect(state, &text_placeholder, 0xc0c0c0);
	draw_rect(state, &left_border, color);
	draw_rect(state, &right_border, color);
	draw_rect(state, &bottom_border, color);
}

static void draw_windows(wm_t *state) {
	window_node_t *node = state->winlist.start;

	for (; node; node = node->next) {
		draw_window(state, &node->window);
		// TODO: have flag to draw decorations conditionally
		draw_window_decoration(state, &node->window);
	}
}

static void draw_status_bar(wm_t *state) {
	stubby_rect_t bar = (stubby_rect_t) {
		.coord = (stubby_point_t) {
			.x = 0,
			.y = state->info.height - 25,
		},
		.width = state->info.width,
		.height = 25,
	};

	// TODO: replace these with image/text drawing library calls
	stubby_rect_t icon_placeholder = (stubby_rect_t) {
		.coord = (stubby_point_t) {
			.x = 5,
			.y = state->info.height - 20,
		},
		.width = 15,
		.height = 15,
	};

	stubby_rect_t text_placeholder = (stubby_rect_t) {
		.coord = (stubby_point_t) {
			.x = 25,
			.y = state->info.height - 20,
		},
		.width = 60,
		.height = 15,
	};

	stubby_rect_t time_placeholder = (stubby_rect_t) {
		.coord = (stubby_point_t) {
			.x = state->info.width - 65,
			.y = state->info.height - 20,
		},
		.width = 60,
		.height = 15,
	};

	// TODO: more efficient
	draw_rect(state, &bar, 0xc0c0c0);
	draw_rect(state, &icon_placeholder, 0x202020);
	draw_rect(state, &text_placeholder, 0x808080);
	draw_rect(state, &time_placeholder, 0x808080);
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
	// TODO: mouse movement scaling
	int32_t up_x = ev->x /*/ 4*/;
	int32_t up_y = ev->y /*/ 4*/;

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
				pt.x += up_x;
				pt.y -= up_y;
				normalize_point(wm, &pt);
				window_set_pos(&node->window, pt.x, pt.y);

				update_window_region(wm, &node->window);
				break;
			}
		}
	}

	wm->mouse.x += up_x;
	wm->mouse.y -= up_y;

	normalize_point(wm, &wm->mouse);
	update_mouse_region(wm);
}

static void handle_keyboard(wm_t *state, keyboard_event_t *ev) {
	window_node_t *node = state->winlist.end;

	if (!node || !node->window.has_buffer) {
		return;
	}

	// TODO: why not forward the raw KEYBOARD_MSG_EVENT?
	message_t msg = {
		.type = STUBBYWM_KEYBOARD_EVENT,
		.data = {
			ev->character,
			ev->modifiers,
			ev->scancode,
			ev->event,
		},
	};

	c4_msg_send_async(&msg, node->window.to_port);
}

static window_node_t *find_client(wm_t *state, uint32_t id) {
	window_node_t *temp = state->winlist.start;

	for (; temp; temp = temp->next) {
		if (id == temp->window.color) {
			return temp;
		}
	}

	return NULL;
}

static void handle_client_update(wm_t *state, message_t *msg) {
	uint32_t id = msg->data[0];
	window_node_t *node = find_client(state, id);

	if (!node) {
		C4_ASSERT(node != NULL);
		return;
	}

	update_window_region(state, &node->window);
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
		draw_status_bar(state);
		draw_mouse(state);
		draw_framebuffer(state);
		reset_updates(state);

		// XXX: ghetto 60fps limiter, need to account for time spent
		//      redrawing, and adapt sleep time as needed
		usleep(1000000 / 60);
		c4rt_peripheral_wait_event(&msg, state->peripherals);

		do {
			switch (msg.type) {
				case MOUSE_MSG_EVENT:
					mouse_parse_event(&msg, &mev);
					handle_mouse(state, &mev);
					break;

				case KEYBOARD_MSG_EVENT:
					keyboard_parse_event(&msg, &kev);
					handle_keyboard(state, &kev);
					break;

				case STUBBYWM_UPDATE:
					handle_client_update(state, &msg);
					break;

				default:
					break;
			}

		} while (c4rt_peripheral_have_event(&msg, state->peripherals));
	}
}

// TODO: add a way to pass parameters to threads when creating them
// TODO: also do locking in wm_t struct
static wm_t state;

static void server_handle_new_window(wm_t *wm, message_t *msg, int32_t port) {
	uint32_t x = msg->data[0];
	uint32_t y = msg->data[1];
	uint32_t width = msg->data[2];
	uint32_t height = msg->data[3];

	c4_debug_printf("--- stubbywm: new window request:"
	                "x: %u, y: %u, width: %u, height: %u\n",
	                x, y, width, height);

	size_t size = width * height * 4;
	window_t newwin = window_create(width, height);

	newwin.to_port    = c4_msg_create_async();
	newwin.from_port  = c4_msg_create_async();
	newwin.sync_port  = c4_msg_create_sync();
	newwin.has_buffer = true;

	message_t reply = {
		.type = STUBBYWM_NEW_WINDOW_RESPONSE,
		.data = {
			x,
			y,
			width,
			height,
			newwin.color,
		},
	};

	c4_msg_send(&reply, port);

	C4_ASSERT(c4_memobj_alloc(&newwin.buffer, size, PAGE_READ | PAGE_WRITE));
	c4_cspace_grant(newwin.to_port, port,
	                CAP_ACCESS | CAP_MODIFY | CAP_MULTI_USE | CAP_SHARE);
	c4_cspace_grant(newwin.from_port, port,
	                CAP_ACCESS | CAP_MODIFY | CAP_MULTI_USE | CAP_SHARE);
	c4_cspace_grant(wm->peripherals, port,
	                CAP_ACCESS | CAP_MODIFY | CAP_MULTI_USE | CAP_SHARE);
	c4_cspace_grant(newwin.sync_port, port,
	                CAP_ACCESS | CAP_MODIFY | CAP_MULTI_USE | CAP_SHARE);
	c4_cspace_grant(newwin.buffer.page_obj, port,
	                CAP_ACCESS | CAP_MODIFY | CAP_MULTI_USE | CAP_SHARE);

	window_list_insert(&wm->winlist, &newwin);
	update_window_region(wm, &newwin);
}

static void server_loop(void) {
	while (true) {
		c4_debug_printf("--- stubbywm: server listening...\n");
		message_t msg;
		c4_msg_recieve(&msg, C4_SERV_PORT);
		c4_debug_printf("--- stubbywm: got message...\n");

		if (msg.type != MESSAGE_TYPE_GRANT_OBJECT){
			continue;
		}

		c4_debug_printf("--- stubbywm: waiting for endpoint...\n");
		int32_t temp = msg.data[5];
		C4_ASSERT(msg.data[0] == CAP_TYPE_IPC_SYNC_ENDPOINT);
		c4_msg_recieve(&msg, temp);

		c4_debug_printf("--- stubbywm: handling message %u...\n", msg.type);

		switch (msg.type) {
			case STUBBYWM_NEW_WINDOW:
				c4_debug_printf("--- stubbywm: have new window request\n");
				server_handle_new_window(&state, &msg, temp);
				break;

			default:
				break;
		}

		c4_cspace_remove(C4_CURRENT_CSPACE, temp);
	}
}

int main(int argc, char *argv[]) {
	//wm_t state;
	memset(&state, 0, sizeof(state));

	// initialize icons
	ppm_load(&state.mouse_cursor, cursor_default_ppm);

	// initialize framebuffer
	state.buf_server = nameserver_lookup(C4_NAMESERVER, "/dev/framebuffer");

	for (unsigned i = 0; state.buf_server == 0 && i < 100; i++) {
		state.buf_server = nameserver_lookup(C4_NAMESERVER, "/dev/framebuffer");
	}

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

	nameserver_bind(C4_NAMESERVER, "stubbywm", C4_SERV_PORT);

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

	static uint8_t worker_stack[PAGE_SIZE];

	// TODO: make a library function to handle all of this
	uint32_t tid = c4_create_thread(server_loop, worker_stack + PAGE_SIZE, 0);
	c4_set_capspace(tid, C4_CURRENT_CSPACE);
	c4_set_addrspace(tid, C4_CURRENT_ADDRSPACE);
	c4_set_pager(tid, C4_PAGER);;
	c4_continue_thread(tid);

	event_loop(&state);

	return 0;
}
