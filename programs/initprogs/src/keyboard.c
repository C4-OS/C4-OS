#include <nameserver/nameserver.h>
#include <interfaces/keyboard.h>
#include <c4rt/c4rt.h>
#include <c4/arch/interrupts.h>
#include <stdbool.h>
#include <stdint.h>

enum {
	CODE_ESCAPE,
	CODE_TAB,
	CODE_LEFT_CONTROL,
	CODE_RIGHT_CONTROL,
	CODE_LEFT_SHIFT,
	CODE_RIGHT_SHIFT,
	CODE_END,
};

const char lowercase[] =
	{ '`', CODE_ESCAPE, '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '-',
	  '=', '\b', CODE_TAB, 'q', 'w', 'e', 'r', 't', 'y', 'u', 'i', 'o', 'p',
	  '[', ']', '\n', CODE_LEFT_CONTROL, 'a', 's', 'd', 'f', 'g', 'h', 'j',
	  'k', 'l', ';', '\'', '?', CODE_LEFT_SHIFT, '?', 'z', 'x', 'c', 'v', 'b',
	  'n', 'm', ',', '.', '/', CODE_RIGHT_SHIFT, '_', '_', ' ', '_', '_', '_',
	  '_', '_', '_', '_', '_', '_', '_', '_', '_', '_', '_', '_',
	};

const char uppercase[] =
	{ '~', CODE_ESCAPE, '!', '@', '#', '$', '%', '^', '&', '*', '(', ')', '_',
	  '+', '\b', CODE_TAB, 'Q', 'W', 'E', 'R', 'T', 'Y', 'U', 'I', 'O', 'P',
	  '{', '}', '\n', CODE_LEFT_CONTROL, 'A', 'S', 'D', 'F', 'G', 'H', 'J',
	  'K', 'L', ':', '"', '?', CODE_LEFT_SHIFT, '?', 'Z', 'X', 'C', 'V', 'B',
	  'N', 'M', '<', '>', '?', CODE_RIGHT_SHIFT, '_', '_', ' ', '_', '_', '_',
	  '_', '_', '_', '_', '_', '_', '_', '_', '_', '_', '_', '_',
	};

static inline bool is_modifier( char c ){
	return c < CODE_END;
}

unsigned get_scancode( void ){
	message_t msg;

	c4_msg_recieve_async( &msg, MESSAGE_ASYNC_BLOCK );

	return c4_in_byte( 0x60 );
}

char decode_scancode( unsigned long code ){
	char ret = '\0';
	static bool is_uppercase = false;
	bool is_keyup = !!(code & 0x80);
	char c;

	code &= ~0x80;
	c = is_uppercase? uppercase[code] : lowercase[code];

	switch ( c ){
		case CODE_LEFT_SHIFT:
		case CODE_RIGHT_SHIFT:
			is_uppercase = !is_keyup;

		default:
			ret = c;
			break;
	}

	return ret;
}

static unsigned modifiers = 0;

void get_event( keyboard_event_t *ev ){
	ev->scancode = get_scancode();
	ev->event    = (ev->scancode & 0x80) ? KEYBOARD_EVENT_KEY_UP
	                                     : KEYBOARD_EVENT_KEY_DOWN;
	char c = decode_scancode( ev->scancode );

	if ( is_modifier( c )){
		switch ( ev->event ){
			case KEYBOARD_EVENT_KEY_DOWN: modifiers |=   1 << c;  break;
			case KEYBOARD_EVENT_KEY_UP:   modifiers &= ~(1 << c); break;
		}
	}

	ev->character = c;
	ev->modifiers = modifiers;
}

void handle_get_event( message_t *request ){
	keyboard_event_t event;
	get_event( &event );

	message_t msg = {
		.type = KEYBOARD_MSG_EVENT,
		.data = {
			event.character,
			event.modifiers,
			event.scancode,
			event.event,
		},
	};

	c4_msg_send( &msg, request->sender );
}

void _start( uintptr_t nameserv ){
	nameserver_bind( nameserv, "/dev/keyboard" );

	message_t msg = {
		.type = MESSAGE_TYPE_INTERRUPT_SUBSCRIBE,
		.data = { INTERRUPT_KEYBOARD, },
	};

	// read input port in case there was a keypress interrupt before
	// the driver had a chance to handle it
	c4_in_byte( 0x60 );
	c4_msg_send( &msg, 0 );

	while ( true ){
		message_t msg;
		c4_msg_recieve( &msg, 0 );

		switch ( msg.type ){
			case KEYBOARD_MSG_GET_EVENT:
				handle_get_event( &msg );
				break;

			default:
				break;
		}
	}
}
