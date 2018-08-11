#ifndef _C4RT_MEM_H
#define _C4RT_MEM_H 1

#include <c4rt/interface/pager.h>
#include <c4/capability.h>

typedef struct mem_object {
	union {
		uintptr_t vaddr;
		void *vaddrptr;
	};

	uint32_t page_obj;
	uint32_t permissions;
	bool mapped;
} c4_mem_object_t;

static inline unsigned c4_page_to_cap_perms( unsigned page_perms ){
	unsigned ret = CAP_MULTI_USE | CAP_SHARE;

	if ( page_perms & PAGE_READ )  ret |= CAP_ACCESS;
	if ( page_perms & PAGE_WRITE ) ret |= CAP_MODIFY;

	return ret;
}

// these functions are intended to be the general high-level memory management
// interfaces when you don't particularly care what address the memory is at
bool c4_memobj_alloc(c4_mem_object_t *memobj, size_t size, unsigned permissions);
void c4_memobj_free(c4_mem_object_t *memobj);

c4_mem_object_t c4_memobj_make( uint32_t obj, uintptr_t vaddr, unsigned permissions );
bool c4_memobj_map( c4_mem_object_t *obj, uint32_t addrspace );
bool c4_memobj_unmap( c4_mem_object_t *obj, uint32_t addrspace );
bool c4_memobj_region_map( uint32_t obj,
                           c4_mem_object_t *memobj,
                           size_t size,
                           unsigned permissions );
bool c4_memobj_region_unmap(c4_mem_object_t *memobj);

//bool            c4_memobj_map( c4_mem_object_t *obj );
//bool            c4_memobj_unmap( c4_mem_object_t *obj );
void            c4_memobj_destroy( c4_mem_object_t *obj );
int             c4_memobj_share( c4_mem_object_t *memobj, uint32_t endpoint );
c4_mem_object_t c4_memobj_recieve( uintptr_t vaddr, uint32_t endpoint );

#endif
