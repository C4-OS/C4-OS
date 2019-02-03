#ifndef _C4OS_STUBBYWM_H
#define _C4OS_STUBBYWM_H 1

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>
#include <c4rt/interface/framebuffer.h>
#include <c4rt/c4rt.h>

enum { MAX_UPDATES = 128, };

typedef struct wm wm_t;
typedef struct point stubby_point_t;

#include <stubbywm/ppm.h>

typedef struct point {
	int32_t x, y;
} stubby_point_t;

typedef struct rect {
	stubby_point_t coord;
	unsigned height;
	unsigned width;
} stubby_rect_t;

// TODO: generic linked list implementation in c4rt
typedef struct window window_t;
typedef struct window_node window_node_t;
typedef struct window_list window_list_t;

struct window {
	stubby_rect_t rect;
	c4_mem_object_t buffer;
	uint32_t color;
	bool has_buffer;

	uint32_t to_port;
	uint32_t from_port;
	uint32_t sync_port;
};

struct window_node {
	window_t window;

	window_node_t *next;
	window_node_t *prev;
};

struct window_list {
	window_node_t *start;
	window_node_t *end;

	size_t nodes;
};

typedef struct wm_update {
	stubby_point_t lower;
	stubby_point_t upper;
} wm_update_t;

typedef struct wm {
	// window things
	window_list_t winlist;

	// peripheral stuff
	uint32_t peripherals;
	stubby_point_t mouse;
	ppm_t mouse_cursor;

	// screen updating structures
	wm_update_t updates[MAX_UPDATES];
	unsigned num_updates;

	// display buffers
	c4_mem_object_t buffer;
	uint32_t buf_cap;
	uint32_t buf_server;
	uint8_t *framebuffer;
	framebuffer_info_t info;
} wm_t;

void draw_pixel(wm_t *state, int32_t x, int32_t y, uint32_t pixel);
void draw_rect(wm_t *state, stubby_rect_t *rect, uint32_t color);

window_node_t *window_list_insert(window_list_t *list, window_t *win);
window_node_t *window_list_find(window_list_t *list, window_t win);
void window_list_remove(window_list_t *list, window_node_t *node);

window_t window_create(int32_t width, int32_t height);
void     window_set_pos(window_t *win, int32_t x, int32_t y);

#endif
