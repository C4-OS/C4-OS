#include <c4rt/c4rt.h>
#include <c4rt/stublibc.h>

char buffer[32];

int main( int argc, char *argv[], char *envp[] ){
	if ( argc <= 0 ){
		return 1;
	}

	char *progname = argv[0];
	c4_debug_printf( "--- %s: %u args\n", progname, argc );


	FILE *foo = fopen( "/sbin/////keyboard", "r" );
	c4_debug_printf( "--- %s: got %p\n", progname, foo );

	for ( unsigned k = 0; k < 4; k++ ){
		size_t nread = fread( buffer, sizeof(buffer), 1, foo );

		for ( unsigned i = 0; i < nread; i++ ){
			c4_debug_printf( "%x ", (uint8_t)buffer[i] );
		}

		c4_debug_printf( "\n" );
	}

	for ( unsigned i = 0; i < 16; i++ ){
		int c = fgetc( foo );
		c4_debug_printf( "--- clibtest: read a character: %x\n", (uint8_t)c );
	}

	/*
	for ( int c = fgetc(foo); !feof(foo); c = fgetc(foo)) {
		c4_debug_printf( "--- clibtest: read a character: %x\n", (uint8_t)c );
	}
	*/

	fclose( foo );

	return 0;
}
