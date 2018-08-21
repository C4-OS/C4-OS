#ifndef _C4OS_MOUSE_INTERFACE_H
#define _C4OS_MOUSE_INTERFACE_H 1
#include <c4rt/interface/peripheral.h>

enum {
	MOUSE_FLAG_LEFT   = (1 << 0),
	MOUSE_FLAG_RIGHT  = (1 << 1),
	MOUSE_FLAG_MIDDLE = (1 << 2),
};

typedef struct mouse_event {
	uint32_t flags;
	int32_t  x;
	int32_t  y;
} mouse_event_t;

static inline void mouse_parse_event(message_t *msg, mouse_event_t *ev) {
    C4_ASSERT(msg->type == MOUSE_MSG_EVENT);

    ev->flags = msg->data[0];
    ev->x     = msg->data[1];
    ev->y     = msg->data[2];
};

#endif
