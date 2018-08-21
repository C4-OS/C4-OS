#ifndef _C4OS_PERIPHERAL_INTERFACE_H
#define _C4OS_PERIPHERAL_INTERFACE_H 1

enum {
    PERIPHERAL_DISCONNECT = 0xd00f,
};

enum {
	KEYBOARD_MSG_GET_EVENT      = MESSAGE_TYPE_END_RESERVED,
	KEYBOARD_MSG_GET_MODIFIERS,
	KEYBOARD_MSG_EVENT,
};

enum {
    MOUSE_MSG_EVENT = KEYBOARD_MSG_EVENT + 1,
};

static inline bool c4rt_peripheral_connect(uint32_t server, uint32_t port) {
	int r = c4_cspace_grant(port, server, CAP_MODIFY| CAP_MULTI_USE| CAP_SHARE);

    return r >= 0;
}

static inline void c4rt_peripheral_disconnect(uint32_t port) {
    // TODO: fill this in
}

static inline void c4rt_peripheral_wait_event(message_t *msg, uint32_t port) {
    c4_msg_recieve_async(msg, port, MESSAGE_ASYNC_BLOCK);
}

#endif
