#include <c4rt/c4rt.h>
#include <c4rt/mem.h>
#include <c4/error.h>

c4_mem_object_t c4_memobj_make( uint32_t obj,
                                uintptr_t vaddr,
                                unsigned permissions )
{
	c4_mem_object_t ret = {
		.vaddr       = vaddr,
		.permissions = permissions,
		.page_obj    = obj,
		.mapped      = false,
	};

	return ret;
}

bool c4_memobj_map( c4_mem_object_t *obj, uint32_t addrspace ){
	if ( obj->mapped )
		return true;

	int ret = c4_addrspace_map( addrspace,
	                            obj->page_obj,
	                            obj->vaddr,
	                            obj->permissions );

	obj->mapped = ret >= 0;
	return obj->mapped;
}

bool c4_memobj_unmap( c4_mem_object_t *obj, uint32_t addrspace ){
	if ( !obj->mapped )
		return false;

	c4_addrspace_unmap( addrspace, obj->vaddr );

	obj->mapped = false;
	return obj->mapped;
}

void c4_memobj_destroy( c4_mem_object_t *obj ){
	c4_cspace_remove( C4_CURRENT_CSPACE, obj->page_obj );
}

int c4_memobj_share( c4_mem_object_t *memobj, uint32_t endpoint ){
	message_t foo = {
		// TODO: Figure out a way to identify types of objects being sent
		//       so that objects can be handled dynamically
		.type = 0xdeadbeef,
		.data = { memobj->permissions },
	};

	c4_msg_send( &foo, endpoint );
	int temp;
	// TODO: either add a way to forward all of the current permissions of the
	//       object to the grant syscall in the kernel, or add a way to get
	//       the current permissions of the object
	unsigned perms = c4_page_to_cap_perms( memobj->permissions );
	temp = c4_cspace_grant( memobj->page_obj, endpoint, perms );

	if ( temp < 0 ){
		return temp;
	}

	temp = c4_cspace_grant( memobj->page_obj, endpoint,
	                        CAP_ACCESS | CAP_MODIFY | CAP_MULTI_USE | CAP_SHARE );

	if ( temp < 0 ){
		return temp;
	}

	return C4_ERROR_NONE;
}

c4_mem_object_t c4_memobj_recieve( uintptr_t vaddr, uint32_t endpoint ){
	message_t msg;
	c4_mem_object_t ret = { .vaddr = vaddr };

	c4_msg_recieve( &msg, endpoint );
	C4_ASSERT( msg.type == 0xdeadbeef );
	ret.permissions = msg.data[0];

	c4_msg_recieve( &msg, endpoint );
	C4_ASSERT( msg.type == MESSAGE_TYPE_GRANT_OBJECT );
	ret.page_obj = msg.data[5];

	return ret;
}

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
