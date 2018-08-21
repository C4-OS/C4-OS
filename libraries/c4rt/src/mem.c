#include <c4rt/c4rt.h>
#include <c4rt/mem.h>
#include <c4rt/addrman.h>
#include <c4/error.h>

// TODO: split up headers, this is just needed for get_genregion()
#include <c4rt/stublibc.h>

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

// TODO: documentation
//       This takes a page capability and maps it into an available space
//       in the general-purpose region, then wraps all the info in a
//       c4_mem_object_t and maps it into the current address space.
//       In other words, wrapping some common operations up into a streamlined
//       interface.
//
//       returns true if the page was successfully mapped, false otherwise.
bool c4_memobj_region_map( uint32_t obj,
                           c4_mem_object_t *memobj,
                           size_t size,
                           unsigned permissions )
{
	if (!get_genregion()) {
		// general-purpose region wasn't initialized for whatever reason,
		// we can't continue
		return false;
	}

	void *ptr = c4rt_vaddr_alloc(get_genregion(), size);

	if (!ptr) {
		return false;
	}

	*memobj = c4_memobj_make(obj, (uintptr_t)ptr, permissions);
	return c4_memobj_map(memobj, C4_CURRENT_ADDRSPACE);
}

bool c4_memobj_region_unmap(c4_mem_object_t *memobj){
	if (!get_genregion()) {
		// general-purpose region wasn't initialized for whatever reason,
		// we can't continue
		return false;
	}

	c4_memobj_unmap(memobj, C4_CURRENT_ADDRSPACE);
	c4rt_vaddr_free(get_genregion(), (void*)memobj->vaddr);

	return true;
}

bool c4_memobj_alloc(c4_mem_object_t *memobj,
                     size_t size,
                     unsigned permissions)
{
	int n_pages = pager_size_to_pages(size);
	int32_t page = pager_request_pages(C4_PAGER, permissions, n_pages);

	return c4_memobj_region_map(page, memobj, size, permissions);
}

void c4_memobj_free(c4_mem_object_t *memobj){
	c4_memobj_region_unmap(memobj);
	c4_memobj_destroy(memobj);
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

// TODO: note in docs that size is in pages
int c4_phys_frame_create( uintptr_t physical, size_t size, uint32_t context ){
	int ret;
	DO_SYSCALL(SYSCALL_PHYS_FRAME_CREATE, physical, size, context, 0, ret);
	return ret;
}

int c4_phys_frame_split( uint32_t frame, size_t offset ){
	int ret;
	DO_SYSCALL( SYSCALL_PHYS_FRAME_SPLIT, frame, offset, 0, 0, ret );
	return ret;
}

uint32_t c4_request_physical( uintptr_t virt,
                              uintptr_t physical,
                              unsigned size,
                              unsigned permissions )
{
	int32_t frame = c4_phys_frame_create(physical, size, 0 /* TODO: C4_CONTEXT */);
	if (frame < 0) {
		return 0;
	}

	int k = c4_addrspace_map(C4_CURRENT_ADDRSPACE, frame, virt, permissions);

	if (k < 0) {
		return 0;
	}

	return frame;
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
