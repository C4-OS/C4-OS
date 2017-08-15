#include <c4rt/c4rt.h>

int c4_addrspace_create( void ){
	int ret;
	DO_SYSCALL( SYSCALL_ADDRSPACE_CREATE, 0, 0, 0, 0, ret );
	return ret;
}

int c4_addrspace_map( uint32_t addrspace,
                      uint32_t frame,
                      uintptr_t address,
                      unsigned permissions )
{
	int ret;
	DO_SYSCALL( SYSCALL_ADDRSPACE_MAP, addrspace,
	            frame, address, permissions, ret );
	return ret;
}

int c4_addrspace_unmap( uint32_t addrspace, uintptr_t address ){
	int ret;
	DO_SYSCALL( SYSCALL_ADDRSPACE_UNMAP, addrspace, address, 0, 0, ret );
	return ret;
}

int c4_phys_frame_split( uint32_t frame, size_t offset ){
	int ret;
	DO_SYSCALL( SYSCALL_PHYS_FRAME_SPLIT, frame, offset, 0, 0, ret );
	return ret;
}

// Deprecated
void *c4_request_physical( uintptr_t virt,
                           uintptr_t physical,
                           unsigned size,
                           unsigned permissions )
{
	return NULL;
}

// Deprecated
int c4_mem_unmap( unsigned thread_id, void *addr ){
	return 0;
}

// Deprecated
int c4_mem_map_to( unsigned thread_id,
                   void *from,
                   void *to,
                   unsigned size,
                   unsigned permissions )
{
	return 0;
}

// Deprecated
int c4_mem_grant_to( unsigned thread_id,
                     void *from,
                     void *to,
                     unsigned size,
                     unsigned permissions )
{
	return 0;
}

// Deprecated
void c4_dump_maps( unsigned thread ){
}
