#include <c4rt/c4rt.h>
#include <stdarg.h>
#include <string.h>

const char *hexadecimal = "0123456789abcdef";
const char *decimal     = "0123456789";
const char *binary      = "01";

void c4_debug_putchar( char c ){
	/*
	message_t msg = {
		.type = MESSAGE_TYPE_DEBUG_PUTCHAR,
		.data = { c },
	};
	*/

	//c4_msg_send( &msg, 0 );
	int ret = 0;

	DO_SYSCALL( SYSCALL_DEBUG_PUTCHAR, c, 0, 0, 0, ret );
}

void c4_debug_puts( const char *str ){
	for ( unsigned i = 0; str[i]; i++ ){
		c4_debug_putchar( str[i] );
	}
}

void c4_debug_print_num( unsigned long n, const char *base_str ){
	char stack[33];
	unsigned base = strlen(base_str);
	unsigned i;

	if ( n == 0 || base == 0 ){
		c4_debug_putchar( '0' );
		return;
	}

	for ( i = 0; n; i++ ){
		stack[i] = base_str[n % base];
		n /= base;
	}

	while ( i ){
		c4_debug_putchar( stack[--i] );
	}
}

void c4_debug_printf( const char *format, ... ){
	va_list args;
	va_start( args, format );

	for ( unsigned i = 0; format[i]; i++ ){
		if ( format[i] == '%' ){
			switch( format[++i] ){
				case 's':
					c4_debug_puts( va_arg( args, char* ));
					break;

				case 'u':
					c4_debug_print_num( va_arg(args, unsigned), decimal );
					break;

				case 'x':
					c4_debug_print_num( va_arg(args, unsigned), hexadecimal );
					break;

				case 'b':
					c4_debug_print_num( va_arg(args, unsigned), binary );
					break;

				case 'p':
					c4_debug_puts( "0x" );
					c4_debug_print_num( va_arg(args, uintptr_t), hexadecimal );
					break;

				default:
					break;
			}

		} else {
			c4_debug_putchar( format[i] );
		}
	}
	
	va_end( args );
}
