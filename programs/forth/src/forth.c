#include <c4rt/c4rt.h>
#include <miniforth/stubs.h>
#include <miniforth/miniforth.h>
#include <stdint.h>
#include <nameserver/nameserver.h>
#include <interfaces/console.h>
#include <interfaces/keyboard.h>

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

static char *read_line( char *buf, unsigned n ){
	unsigned i = 0;

	for ( i = 0; i < n - 1; i++ ){
		char c = 0;
retry:
		{
			keyboard_event_t ev;
			keyboard_get_event( keyboard, &ev );

			if ( keyboard_event_is_modifier( &ev ))
				goto retry;

			if ( ev.event != KEYBOARD_EVENT_KEY_DOWN )
				goto retry;

			c = ev.character;
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
