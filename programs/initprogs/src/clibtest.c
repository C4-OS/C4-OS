#include <c4rt/c4rt.h>
#include <c4rt/stublibc.h>

int main( int argc, char *argv[], char *envp[] ){
	if ( argc <= 0 ){
		return 1;
	}

	char *progname = argv[0];
	c4_debug_printf( "--- %s: %u args\n", progname, argc );

	char buffer[128];

	FILE *foo = fopen( "/sbin/////forth", "r" );
	c4_debug_printf( "--- %s: got %p\n", progname, foo );
	size_t nread = fread( buffer, 128, 1, foo );

	for ( unsigned i = 0; i < nread; i++ ){
		c4_debug_printf( "%x ", (uint8_t)buffer[i] );
	}

	c4_debug_printf( "\n" );
	fclose( foo );

	return 0;
}
