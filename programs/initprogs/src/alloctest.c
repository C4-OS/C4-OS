#include <c4rt/c4rt.h>
#include <c4alloc/c4alloc.h>

static c4a_heap_t heapy;

void _start( unsigned long nameserver ){
	c4a_heap_init( &heapy, 0xbeef0000 );

	unsigned **test = c4a_alloc( &heapy, sizeof( unsigned *[64] ));

	for ( unsigned k = 0; k < 64; k++ ){
		test[k]  = c4a_alloc( &heapy, 32 );
		c4_debug_printf( "--- alloctest: allocated %p\n", test[k] );
		*test[k] = 0xcafebeef;
	}

	for ( unsigned k = 0; k < 64; k += 2 ){
		c4a_free( &heapy, test[k] );
	}

	for ( unsigned k = 1; k < 64; k += 2 ){
		c4_debug_printf( "--- alloctest: testing: %p -> %p\n", test[k], *test[k] );
	}

	c4a_heap_deinit( &heapy );
	c4_exit();
}
