#include <c4rt/c4rt.h>
#include <miniforth/stubs.h>
#include <miniforth/miniforth.h>
#include <stdint.h>
#include <nameserver/nameserver.h>
#include <interfaces/console.h>

static unsigned display = 0;
static unsigned keyboard = 0;

static void putchar( char c ){
	console_put_char( display, c );
}

static void debug_print( const char *s ){
	for ( ; *s; s++ ){
		putchar(*s);
	}
}

/*
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
*/


enum {
	CODE_ESCAPE,
	CODE_TAB,
	CODE_LEFT_CONTROL,
	CODE_RIGHT_CONTROL,
	CODE_LEFT_SHIFT,
	CODE_RIGHT_SHIFT,
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

char decode_scancode( unsigned long code ){
	static bool is_uppercase = false;
	char c = is_uppercase? uppercase[code] : lowercase[code];
	char ret = '\0';

	switch ( c ){
		case CODE_LEFT_SHIFT:
		case CODE_RIGHT_SHIFT:
			is_uppercase = !is_uppercase;
			break;

		default:
			ret = c;
			break;
	}

	return ret;
}

// XXX: old read line from sigma0, used until a proper console/display program
//      to multiplex the display and peripherals is written
static char *read_line( char *buf, unsigned n ){
	message_t msg;
	unsigned i = 0;

	for ( i = 0; i < n - 1; i++ ){
		char c = 0;
retry:
		{
			// XXX: this relies on the /bin/keyboard program being started,
			//      which itself is initialized by the forth interpreter,
			//      so this assumes that the program will be started by the
			//      init_commands.fs script before entering interactive mode
			msg.type = 0xbadbeef;
			c4_msg_send( &msg, keyboard );
			c4_msg_recieve( &msg, keyboard );

			c = decode_scancode( msg.data[0] );

			if ( !c || msg.data[1] != 0 ){
				goto retry;
			}
		}

		console_put_char( display, c );

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
			"\"hellow, world!\" pstring cr\n"
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
	console_put_char( display, c );
}

void add_c4_archives( minift_vm_t *vm );
void init_c4_allocator( minift_vm_t *vm );

void _start( uintptr_t nameserver ){
	unsigned long data[512];
	unsigned long calls[32];
	unsigned long params[32];

	// TODO: implement a better way of waiting for devices to avoid polling
	while ( !display ){
		display  = nameserver_lookup( nameserver, "/dev/console" );
	}

	while ( !keyboard ){
		keyboard = nameserver_lookup( nameserver, "/dev/keyboard" );
	}

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
	init_c4_allocator( &foo );
	add_c4_archives( &foo );
	minift_run( &foo );

	c4_exit();
}
