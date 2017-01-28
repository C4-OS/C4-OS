#include <c4rt/c4rt.h>
#include <stdint.h>

static void putchar( char c ){
	unsigned display = 3;

	message_t msg = {
		.type = 0xbabe,
		.data = { c },
	};

	c4_msg_send( &msg, display );
}

static void puts( const char *s ){
	for ( ; *s; s++ ){
		putchar(*s);
	}
}

void _start( uintptr_t display ){
	puts( "Hello, cool world!\n" );

	c4_exit();
}
