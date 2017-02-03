#include <c4rt/c4rt.h>
#include <c4alloc/c4alloc.h>

static c4a_heap_t heapy;

enum {
	ALLOC_SMALL_SIZE = 128,
	ALLOC_ITERS      = 32,
};

void _start( unsigned long nameserver ){
	c4a_heap_init( &heapy, 0xbeef0000 );

	unsigned **test = c4a_alloc( &heapy, sizeof( unsigned *[ALLOC_ITERS] ));

	for ( unsigned k = 0; k < ALLOC_ITERS; k++ ){
		test[k]  = c4a_alloc( &heapy, ALLOC_SMALL_SIZE );
		c4_debug_printf( "--- alloctest: allocated %p\n", test[k] );
		*test[k] = 0xcafebeef;
	}

	for ( unsigned k = 1; k < ALLOC_ITERS; k += 2 ){
		c4a_free( &heapy, test[k] );
	}

	for ( unsigned k = 0; k < ALLOC_ITERS; k += 2 ){
		c4a_free( &heapy, test[k] );
	}

	for ( unsigned k = 0; k < ALLOC_ITERS; k++ ){
		test[k]  = c4a_alloc( &heapy, ALLOC_SMALL_SIZE );
		c4_debug_printf( "--- alloctest: allocated %p\n", test[k] );
		for ( unsigned j = 0; j < 3; j++ ){
			*(test[k] + j) = 0xcafebeef;
		}
	}

	for ( unsigned k = 0; k < ALLOC_ITERS; k++ ){
		c4_debug_printf( "--- alloctest: freeing %p\n", test[k] );
		c4a_free( &heapy, test[k] );
	}

	c4a_free( &heapy, test );

	c4a_heap_deinit( &heapy );
	c4_exit();
}
