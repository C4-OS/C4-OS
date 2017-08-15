#include <c4rt/c4rt.h>

int c4_msg_create_sync( void ){
	int ret = 0;

	DO_SYSCALL( SYSCALL_SYNC_CREATE, 0, 0, 0, 0, ret );

	return ret;
}

int c4_msg_send( message_t *buffer, unsigned to ){
	int ret = 0;

	DO_SYSCALL( SYSCALL_SYNC_SEND, buffer, to, 0, 0, ret );

	return ret;
}

int c4_msg_recieve( message_t *buffer, unsigned from ){
	int ret = 0;

	DO_SYSCALL( SYSCALL_SYNC_RECIEVE, buffer, from, 0, 0, ret );

	return ret;
}

int c4_msg_create_async( void ){
	int ret = 0;

	DO_SYSCALL( SYSCALL_SYNC_CREATE, 0, 0, 0, 0, ret );

	return ret;
}

int c4_msg_send_async( message_t *buffer, unsigned to ){
	int ret = 0;

	DO_SYSCALL( SYSCALL_ASYNC_SEND, buffer, to, 0, 0, ret );

	return ret;
}

int c4_msg_recieve_async( message_t *buffer, unsigned flags ){
	int ret = 0;

	DO_SYSCALL( SYSCALL_ASYNC_RECIEVE, buffer, flags, 0, 0, ret );

	return ret;
}
