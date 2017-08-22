#include <c4rt/c4rt.h>

// capability functions
int c4_cspace_create( void ){
	int ret;
	DO_SYSCALL( SYSCALL_CSPACE_CREATE, 0, 0, 0, 0, ret );
	return ret;
}

int c4_cspace_move( uint32_t src_space, uint32_t src_object,
                    uint32_t dest_space, uint32_t dest_object )
{
	int ret;
	DO_SYSCALL( SYSCALL_CSPACE_CAP_MOVE, src_space,
	            src_object, dest_space, dest_object, ret );
	return ret;
}

int c4_cspace_copy( uint32_t src_space, uint32_t src_object,
                    uint32_t dest_space, uint32_t dest_object )
{
	int ret;
	DO_SYSCALL( SYSCALL_CSPACE_CAP_COPY, src_space,
	            src_object, dest_space, dest_object, ret );
	return ret;
}


int c4_cspace_remove( uint32_t cspace, uint32_t object ){
	int ret;
	DO_SYSCALL( SYSCALL_CSPACE_CAP_REMOVE, cspace, object, 0, 0, ret );
	return ret;
}

int c4_cspace_restrict( uint32_t cspace, uint32_t object, unsigned perms ){
	int ret;
	DO_SYSCALL( SYSCALL_CSPACE_CAP_RESTRICT, cspace, object, perms, 0, ret );
	return ret;
}

int c4_cspace_grant( uint32_t object, uint32_t queue, unsigned perms ){
	int ret;
	DO_SYSCALL( SYSCALL_CSPACE_CAP_GRANT, object, queue, perms, 0, ret );
	return ret;
}
