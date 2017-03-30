#include <c4rt/c4rt.h>
#include <c4rt/stublibc.h>

int main( int argc, char *argv[], char *envp[] ){
	if ( argc <= 0 ){
		return 1;
	}

	char *progname = argv[0];
	c4_debug_printf( "--- %s: %u args\n", progname, argc );

	FILE *foo = fopen( "/sbin/////forth", "r" );

	c4_debug_printf( "--- %s: got %p\n", progname, foo );

	return 0;
}
