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

typedef struct keyboard {
	uint32_t server;
	uint32_t endpoint;
} keyboard_t;


static inline bool keyboard_event_is_modifier( keyboard_event_t *ev ){
	return ev->character < KEYBOARD_MOD_END;
}

static inline bool keyboard_connect(keyboard_t *kbd, uint32_t server) {
	kbd->endpoint = c4_msg_create_sync();
	kbd->server = server;

	return true;
}

static inline void keyboard_disconnect(keyboard_t *kbd) {
	c4_cspace_remove(C4_CURRENT_CSPACE, kbd->endpoint);
}

static inline void keyboard_call(keyboard_t *kbd, message_t *msg) {
	c4_cspace_grant(kbd->endpoint, kbd->server,
	                CAP_ACCESS | CAP_MODIFY | CAP_MULTI_USE | CAP_SHARE);
	c4_msg_send(msg, kbd->endpoint);
	c4_msg_recieve(msg, kbd->endpoint);
}

static inline void keyboard_get_event(keyboard_t *kbd, keyboard_event_t *ev) {
	message_t msg = { .type = KEYBOARD_MSG_GET_EVENT, };

	keyboard_call(kbd, &msg);
	C4_ASSERT(msg.type == KEYBOARD_MSG_EVENT);

	ev->character = msg.data[0];
	ev->modifiers = msg.data[1];
	ev->scancode  = msg.data[2];
	ev->event     = msg.data[3];
}

static inline unsigned keyboard_get_modifiers(keyboard_t *kbd) {
	message_t msg = { .type = KEYBOARD_MSG_GET_MODIFIERS, };

	keyboard_call(kbd, &msg);
	return msg.data[0];
}

#endif
