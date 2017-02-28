#include <c4rt/c4rt.h>
#include <nameserver/nameserver.h>
#include <interfaces/block.h>
#include <c4/paging.h>
#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

uint8_t buffer[PAGE_SIZE] __attribute__((aligned(PAGE_SIZE)));

void _start( unsigned nameserver ){
	c4_debug_printf( "--- atatest: got here\n" );
	unsigned ata = 0;

	while ( !ata ){
		ata = nameserver_lookup( nameserver, "/dev/ata" );
	}

	c4_debug_printf( "--- atatest: have ata server at %u\n", ata );

	c4_debug_printf( "--- atatest: reading sector 0...\n" );
	bool had_error = block_read( ata, buffer, 0, 0, 2 );

	if ( had_error ){
		c4_debug_printf( "--- atatest: had error while reading\n" );

	} else {
		c4_debug_printf( "--- atatest: done, got: " );

		for ( unsigned i = 0; i < 1024; i++ ){
			c4_debug_printf( "%x ", buffer[i] );
		}

		c4_debug_printf( "\n" );
	}

	c4_exit();
}
