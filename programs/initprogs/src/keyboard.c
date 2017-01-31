#include <c4rt/c4rt.h>
#include <c4/arch/interrupts.h>
#include <stdbool.h>
#include <stdint.h>

enum {
	NAME_BIND = 0x1024,
	NAME_UNBIND,
	NAME_LOOKUP,
	NAME_RESULT,
};

unsigned hash_string( const char *str ){
	unsigned hash = 757;
	int c;

	while (( c = *str++ )){
		hash = ((hash << 7) + hash + c);
	}

	return hash;
}

static inline void nameserver_bind( unsigned server, unsigned long name ){
	message_t msg = {
		.type = NAME_BIND,
		.data = { name },
	};

	c4_msg_send( &msg, server );
}

static inline unsigned nameserver_lookup( unsigned server, unsigned long name ){
	message_t msg = {
		.type = NAME_LOOKUP,
		.data = { name },
	};

	c4_msg_send( &msg, server );
	c4_msg_recieve( &msg, server );

	return msg.data[0];
}

unsigned get_scancode( void ){
	message_t msg;

	c4_msg_recieve_async( &msg, MESSAGE_ASYNC_BLOCK );

	return c4_in_byte( 0x60 );
}

void _start( uintptr_t data ){
	uintptr_t nameserv = data;

	nameserver_bind( nameserv, hash_string( "/dev/keyboard" ));

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

		// wait for a keystroke request
		c4_msg_recieve( &msg, 0 );

		if ( msg.type != 0xbadbeef )
			continue;

		unsigned scancode = get_scancode( );
		bool     key_up   = !!(scancode & 0x80);

		scancode &= ~0x80;

		msg.type = 0xbeef;
		msg.data[0] = scancode;
		msg.data[1] = key_up;

		c4_msg_send( &msg, msg.sender );
	}
}
