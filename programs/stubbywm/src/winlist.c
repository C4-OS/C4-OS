#include <c4rt/c4rt.h>
#include <c4rt/prng.h>
#include <stubbywm/stubbywm.h>

#include <stddef.h>
#include <stdlib.h>

window_node_t *window_list_insert(window_list_t *list, window_t *win) {
	window_node_t *new_node = c4rt_calloc(1, sizeof(window_node_t));
	C4_ASSERT(new_node != NULL);

	new_node->window = *win;

	if (list->end) {
		list->end->next = new_node;
		new_node->prev = list->end;
	}

	if (!list->start) {
		list->start = new_node;
	}

	list->end = new_node;
	list->nodes++;

	return new_node;
}

void window_list_remove(window_list_t *list, window_node_t *node){
	if (node->prev) {
		node->prev->next = node->next;
	}

	if (node->next) {
		node->next->prev = node->prev;
	}

	if (node == list->start) {
		list->start = node->next;
	}

	if (node == list->end) {
		list->end = node->prev;
	}

	c4rt_free(node);
}

window_node_t *window_list_find(window_list_t *list, window_t win) {
	return NULL;
}

window_t window_create(int32_t width, int32_t height){
	return (window_t) {
		.rect = {
			.coord = { 0, 0, },
			.width = width,
			.height = height,
		},

		.color = c4rt_prng_u32(),
		.has_buffer = false,
	};
}

void window_set_pos(window_t *win, int32_t x, int32_t y) {
	win->rect.coord = (stubby_point_t){ x, y };
}
