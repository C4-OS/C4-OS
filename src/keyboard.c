#include <c4rt/c4rt.h>
#include <c4/arch/interrupts.h>
#include <stdbool.h>
#include <stdint.h>

void _start( void *data ){
	uintptr_t display = (uintptr_t)data;

	message_t msg = {
		.type = MESSAGE_TYPE_INTERRUPT_SUBSCRIBE,
		.data = { INTERRUPT_KEYBOARD, },
	};

	c4_msg_send( &msg, 0 );

	while ( true ){
		c4_msg_recieve_async( &msg, MESSAGE_ASYNC_BLOCK );

		unsigned scancode = c4_in_byte( 0x60 );
		bool     key_up   = !!(scancode & 0x80);

		scancode &= ~0x80;

		msg.type = 0xbeef;
		msg.data[0] = scancode;
		msg.data[1] = key_up;

		c4_msg_send( &msg, display );
	}
}
