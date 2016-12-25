#include <c4rt/c4rt.h>

uint8_t c4_inbyte( unsigned port ){
	int ret;

	DO_SYSCALL( SYSCALL_IOPORT, SYSCALL_IO_INPUT, port, 0, 0, ret );

	return ret;
}

void c4_outbyte( unsigned port, uint8_t value ){
	int ret;

	DO_SYSCALL( SYSCALL_IOPORT, SYSCALL_IO_OUTPUT, port, value, 0, ret );

	return;
}
