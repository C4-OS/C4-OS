#include <c4rt/c4rt.h>
#include <stdint.h>
#include <stdbool.h>

typedef struct name_entry {
	unsigned long hash;
	//unsigned long thread;
	int32_t endpoint;
} name_entry_t;

enum {
	MAX_NAME_ENTRIES = 512,
	NAME_BIND = 0x1024,
	NAME_UNBIND,
	NAME_LOOKUP,
	NAME_RESULT,
};

static name_entry_t names[MAX_NAME_ENTRIES];

// TODO: just replace `putchar` with c4_debug_putchar, no need for code
//       duplication
static void putchar( char c ){
	c4_debug_putchar( c );
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

void bind_name( unsigned long endpoint, unsigned long name ){
	unsigned i = 0;

	for ( ; i < MAX_NAME_ENTRIES && names[i].hash; i++ ){
		if ( name == names[i].hash )
			break;
	}

	names[i].endpoint = endpoint;
	names[i].hash     = name;
}

unsigned long lookup_name( unsigned long name ){
	for ( unsigned i = 0; i < MAX_NAME_ENTRIES && names[i].hash; i++ ){
		if ( name == names[i].hash ){
			return names[i].endpoint;
		}
	}

	return 0;
}

void handle_bind( int32_t endpoint, unsigned long hash ){
	bind_name( endpoint, hash );
	c4_debug_printf( "--- nameserver: bound object at %x:%x\n", hash, endpoint );
}

void handle_lookup( int32_t responseq, unsigned long hash ){
	int32_t obj = lookup_name( hash );
	c4_debug_printf( "--- nameserver: looking up object %x:%x\n", hash, responseq );

	c4_cspace_grant( obj, responseq, CAP_MODIFY | CAP_SHARE | CAP_MULTI_USE );
	c4_cspace_remove( 0, responseq );
}

void _start( uintptr_t display ){
	while ( true ){
		message_t msg;
		int32_t obj;
		int ret;

		c4_debug_printf( "--- nameserver: waiting: %u\n", c4_get_id() );
		int k = c4_msg_recieve( &msg, 1 );
		c4_debug_printf( "--- nameserver: got %u\n", k );
		c4_debug_printf( "--- nameserver: recieved message %u\n", msg.type );

		switch ( msg.type ){
			case MESSAGE_TYPE_GRANT_OBJECT:
				obj = msg.data[5];
				ret = c4_msg_recieve( &msg, obj );

				C4_ASSERT( ret >= 0 );
				C4_ASSERT( msg.type == NAME_BIND || msg.type == NAME_LOOKUP );

				if ( msg.type == NAME_BIND ){
					handle_bind( obj, msg.data[0] );

				} else if ( msg.type == NAME_LOOKUP ){
					handle_lookup( obj, msg.data[0] );

				} else {
					c4_debug_printf( "--- nameserver: don't know what to "
					                 "do with object %u\n", obj );
					c4_cspace_remove( 0, obj );
				}

				break;

			case NAME_UNBIND:
				break;

			default:
				c4_debug_printf( "--- nameserver: unknown message %u\n", msg.type );
				break;
		}
	}

	c4_exit();
}
