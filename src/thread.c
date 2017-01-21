#include <c4rt/c4rt.h>

void c4_exit( void ){
	int unused = 0;

	DO_SYSCALL( SYSCALL_EXIT, 0, 0, 0, 0, unused );
}

int c4_create_thread( void *entry, void *stack, unsigned flags ){
	int ret = 0;

	DO_SYSCALL( SYSCALL_CREATE_THREAD, entry, stack, flags, 0, ret );

	return ret;
}

int c4_continue_thread( unsigned thread ){
	message_t buf = { .type = MESSAGE_TYPE_CONTINUE, };

	return c4_msg_send( &buf, thread );
}

int c4_set_pager( unsigned thread, unsigned pager ){
	message_t buf = {
		.type = MESSAGE_TYPE_SET_PAGER,
		.data = { pager },
	};

	return c4_msg_send( &buf, thread );
}
