#include <c4rt/c4rt.h>
#include <miniforth/stubs.h>
#include <miniforth/miniforth.h>
#include <stdint.h>

static unsigned display = 0;

enum {
	NAME_BIND = 0x1024,
	NAME_UNBIND,
	NAME_LOOKUP,
	NAME_RESULT,
};

static void putchar( char c ){
	message_t msg = {
		.type = 0xbabe,
		.data = { c },
	};

	c4_msg_send( &msg, display );
}

static void debug_print( const char *s ){
	for ( ; *s; s++ ){
		putchar(*s);
	}
}

unsigned hash_string( const char *str ){
	unsigned hash = 757;
	int c;

	while (( c = *str++ )){
		hash = ((hash << 7) + hash + c);
	}

	return hash;
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

static char *read_line( char *buf, unsigned n ){
	message_t msg;
	unsigned i = 0;

	for ( i = 0; i < n - 1; i++ ){
retry:
		c4_msg_recieve( &msg, 0 );

		if ( msg.type != 0xbabe )
			goto retry;

		char c = msg.data[0];

		c4_msg_send( &msg, display );

		if ( i && c == '\b' ){
			i--;
			goto retry;
		}

		buf[i] = c;

		if ( c == '\n' ){
			break;
		}
	}

	buf[++i] = '\0';

	return buf;
}

char minift_get_char( void ){
	static char input[80];
	static bool initialized = false;
	static char *ptr;

	if ( !initialized ){
		for ( unsigned i = 0; i < sizeof(input); i++ ){ input[i] = 0; }
		// left here in case some sort of init script is added in the future
		//ptr = input;
		ptr =
			": pstring while dup c@ 0 != begin dup c@ emit 1 + repeat ; "
			"\"hellow, world!\" pstring cr"
		;

		initialized = true;
	}

	while ( !*ptr ){
		debug_print( "miniforth > " );
		ptr = read_line( input, sizeof( input ));
	}

	return *ptr++;
}

void minift_put_char( char c ){
	message_t msg;

	msg.type    = 0xbabe;
	msg.data[0] = c;

	c4_msg_send( &msg, display );
}

void _start( uintptr_t nameserver ){
	unsigned long data[512];
	unsigned long calls[32];
	unsigned long params[32];

	display = 3;

	minift_vm_t foo;
	minift_stack_t data_stack = {
		.start = data,
		.end   = data + 256,
		.ptr   = data,
	};

	minift_stack_t call_stack = {
		.start = calls,
		.end   = calls + 32,
		.ptr   = calls,
	};

	minift_stack_t param_stack = {
		.start = params,
		.end   = params + 32,
		.ptr   = params,
	};

	minift_init_vm( &foo, &call_stack, &data_stack, &param_stack, NULL );
	minift_run( &foo );

	c4_exit();
}
