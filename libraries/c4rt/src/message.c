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

	DO_SYSCALL( SYSCALL_ASYNC_CREATE, 0, 0, 0, 0, ret );

	return ret;
}

int c4_msg_send_async( message_t *buffer, unsigned to ){
	int ret = 0;

	DO_SYSCALL( SYSCALL_ASYNC_SEND, buffer, to, 0, 0, ret );

	return ret;
}

int c4_msg_recieve_async( message_t *buffer, unsigned from, unsigned flags ){
	int ret = 0;

	DO_SYSCALL( SYSCALL_ASYNC_RECIEVE, buffer, from, flags, 0, ret );

	return ret;
}

int c4_send_temp_endpoint( uint32_t server ){
	int32_t temppoint = c4_msg_create_sync();

	C4_ASSERT( temppoint > 0 );
	if ( temppoint < 0 ){
		return temppoint;
	}

	c4_cspace_grant( temppoint, server, CAP_ACCESS | CAP_MODIFY
	                                    | CAP_MULTI_USE | CAP_SHARE );

	return temppoint;
}
