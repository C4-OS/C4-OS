#include <c4rt/c4rt.h>

int c4_create_thread( void *entry, void *stack, unsigned flags ){
	int ret = 0;

	DO_SYSCALL( SYSCALL_CREATE_THREAD, entry, stack, flags, 0, ret );

	return ret;
}

int c4_continue_thread( unsigned thread ){
	message_t buf = { .type = MESSAGE_TYPE_CONTINUE, };

	return c4_msg_send( &buf, thread );
}
