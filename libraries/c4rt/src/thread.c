#include <c4rt/c4rt.h>

void c4_exit( void ){
	int unused = 0;

	DO_SYSCALL( SYSCALL_THREAD_EXIT, 0, 0, 0, 0, unused );
}

int c4_info( unsigned action ){
	int ret = 0;

	DO_SYSCALL( SYSCALL_INFO, action, 0, 0, 0, ret );

	return ret;
}

int c4_create_thread( void *entry, void *stack, unsigned flags ){
	int ret = 0;

	DO_SYSCALL( SYSCALL_THREAD_CREATE, entry, stack, flags, 0, ret );

	return ret;
}

int c4_continue_thread( unsigned thread ){
	int ret = 0;
	DO_SYSCALL( SYSCALL_THREAD_CONTINUE, thread, 0, 0, 0, ret );
	return ret;
}

int c4_set_addrspace( unsigned thread, unsigned space ){
	int ret = 0;
	DO_SYSCALL( SYSCALL_THREAD_SET_ADDRSPACE, thread, space, 0, 0, ret );
	return ret;
}

int c4_set_capspace( unsigned thread, unsigned space ){
	int ret = 0;
	DO_SYSCALL( SYSCALL_THREAD_SET_CAPSPACE, thread, space, 0, 0, ret );
	return ret;
}

int c4_set_pager( unsigned thread, unsigned pager ){
	int ret = 0;
	DO_SYSCALL( SYSCALL_THREAD_SET_PAGER, thread, pager, 0, 0, ret );
	return ret;
}

int c4_get_pager( void ){
	//return c4_info( SYSCALL_INFO_GET_PAGER );
	return -1;
}

int c4_get_id( void ){
	return c4_info( SYSCALL_INFO_GET_ID );
}
