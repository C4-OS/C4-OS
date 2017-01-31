#include <c4rt/c4rt.h>
#include <stdint.h>
#include <stdbool.h>

typedef struct name_entry {
	unsigned long hash;
	unsigned long thread;
} name_entry_t;

enum {
	MAX_NAME_ENTRIES = 512,
	NAME_BIND = 0x1024,
	NAME_UNBIND,
	NAME_LOOKUP,
	NAME_RESULT,
};

static name_entry_t names[MAX_NAME_ENTRIES];

static void putchar( char c ){
	message_t msg = {
		.type = MESSAGE_TYPE_DEBUG_PUTCHAR,
		.data = { c },
	};

	c4_msg_send( &msg, 0 );
}

static void do_puts( const char *s ){
	for ( ; *s; s++ ){
		putchar(*s);
	}
}

static void puts( const char *s ){
	do_puts( "--- nameserver: " );
	do_puts( s );
}

void bind_name( unsigned long thread, unsigned long name ){
	unsigned i = 0;

	for ( ; i < MAX_NAME_ENTRIES && names[i].hash; i++ ){
		if ( name == names[i].hash )
			break;
	}

	names[i].thread = thread;
	names[i].hash   = name;
}

unsigned long lookup_name( unsigned long name ){
	for ( unsigned i = 0; i < MAX_NAME_ENTRIES && names[i].hash; i++ ){
		if ( name == names[i].hash ){
			return names[i].thread;
		}
	}

	return 0;
}

void _start( uintptr_t display ){
	while ( true ){
		message_t msg;

		c4_msg_recieve( &msg, 0 );

		switch ( msg.type ){
			case NAME_BIND:
				puts( "bound a name\n" );
				bind_name( msg.sender, msg.data[0] );
				break;

			case NAME_UNBIND:
				break;

			case NAME_LOOKUP:
				msg.type = NAME_RESULT;
				msg.data[0] = lookup_name( msg.data[0] );
				c4_msg_send( &msg, msg.sender );
				break;
		}
	}

	c4_exit();
}
