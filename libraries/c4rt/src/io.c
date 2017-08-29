#include <c4rt/c4rt.h>

int c4_interrupt_subscribe( unsigned num, uint32_t endpoint ){
	int ret;

	DO_SYSCALL( SYSCALL_INTERRUPT_SUBSCRIBE, num, endpoint, 0, 0, ret );

	return ret;
}

int c4_interrupt_unsubscribe( uint32_t endpoint ){
	int ret;

	DO_SYSCALL( SYSCALL_INTERRUPT_SUBSCRIBE, endpoint, 0, 0, 0, ret );

	return ret;
}

uint8_t c4_in_byte( unsigned port ){
	int ret;

	DO_SYSCALL( SYSCALL_IOPORT, IO_PORT_IN_BYTE, port, 0, 0, ret );

	return ret;
}

uint16_t c4_in_word( unsigned port ){
	int ret;

	DO_SYSCALL( SYSCALL_IOPORT, IO_PORT_IN_WORD, port, 0, 0, ret );

	return ret;
}

uint32_t c4_in_dword( unsigned port ){
	int ret;

	DO_SYSCALL( SYSCALL_IOPORT, IO_PORT_IN_DWORD, port, 0, 0, ret );

	return ret;
}

void c4_out_byte( unsigned port, uint8_t value ){
	int ret;

	DO_SYSCALL( SYSCALL_IOPORT, IO_PORT_OUT_BYTE, port, value, 0, ret );

	return;
}

void c4_out_word( unsigned port, uint16_t value ){
	int ret;

	DO_SYSCALL( SYSCALL_IOPORT, IO_PORT_OUT_WORD, port, value, 0, ret );

	return;
}

void c4_out_dword( unsigned port, uint32_t value ){
	int ret;

	DO_SYSCALL( SYSCALL_IOPORT, IO_PORT_OUT_DWORD, port, value, 0, ret );

	return;
}
