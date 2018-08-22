#ifndef _C4OS_STUBBYWM_H
#define _C4OS_STUBBYWM_H 1

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

typedef struct window {
	stubby_rect_t rect;

	int level;
} window_t;

typedef struct wm {
	c4_mem_object_t buffer;

	uint32_t buf_cap;
	uint32_t buf_server;
	uint8_t *framebuffer;
	framebuffer_info_t info;

	uint32_t peripherals;
	stubby_point_t mouse;
	ppm_t mouse_cursor;

	struct {
		stubby_point_t lower;
		stubby_point_t upper;
	} updates;
} wm_t;

void draw_pixel(wm_t *state, int32_t x, int32_t y, uint32_t pixel);
void draw_rect(wm_t *state, stubby_rect_t *rect, uint32_t color);

#endif
