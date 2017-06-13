#ifndef _C4OS_KEYBOARD_INTERFACE_H
#define _C4OS_KEYBOARD_INTERFACE_H 1
#include <c4rt/c4rt.h>
#include <c4/message.h>

enum {
	KEYBOARD_MSG_GET_EVENT      = MESSAGE_TYPE_END_RESERVED,
	KEYBOARD_MSG_GET_MODIFIERS,
	KEYBOARD_MSG_EVENT,
};

enum {
	KEYBOARD_MOD_ESCAPE,
	KEYBOARD_MOD_TAB,
	KEYBOARD_MOD_LEFT_CONTROL,
	KEYBOARD_MOD_RIGHT_CONTROL,
	KEYBOARD_MOD_LEFT_SHIFT,
	KEYBOARD_MOD_RIGHT_SHIFT,
	KEYBOARD_MOD_END,
};

enum {
	KEYBOARD_EVENT_KEY_DOWN,
	KEYBOARD_EVENT_KEY_UP,
};

typedef struct keyboard_event {
	unsigned character;
	unsigned modifiers;
	unsigned scancode;
	unsigned event;
} keyboard_event_t;

static inline bool keyboard_event_is_modifier( keyboard_event_t *ev ){
	return ev->character < KEYBOARD_MOD_END;
}

static inline void keyboard_get_event( unsigned id, keyboard_event_t *ev ){
	message_t msg = { .type = KEYBOARD_MSG_GET_EVENT, };

	c4_msg_send( &msg, id );
	c4_msg_recieve( &msg, id );
	C4_ASSERT( msg.type == KEYBOARD_MSG_EVENT );

	ev->character = msg.data[0];
	ev->modifiers = msg.data[1];
	ev->scancode  = msg.data[2];
	ev->event     = msg.data[3];
}

static inline unsigned keyboard_get_modifiers( unsigned id ){
	message_t msg = { .type = KEYBOARD_MSG_GET_MODIFIERS, };

	c4_msg_send( &msg, id );
	c4_msg_recieve( &msg, id );

	return msg.data[0];
}

#endif
