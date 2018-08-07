#ifndef _C4RT_ADDRMAN_H
#define _C4RT_ADDRMAN_H 1

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include <c4rt/andtree.h>

typedef struct c4rt_vnode        c4rt_vnode_t;
typedef struct c4rt_vaddr_region c4rt_vaddr_region_t;

struct c4rt_vnode {
	unsigned size;
	unsigned offset;
	bool is_free;
};

struct c4rt_vaddr_region {
	// TODO: add a lock
	
	c4rt_andtree_t layout;
	c4rt_andtree_t free_nodes;

	union {
		void *ptr;
		uintptr_t num;
	} base;
};

c4rt_vaddr_region_t *c4rt_vaddr_region_create( uintptr_t base, unsigned pages );
void *c4rt_vaddr_alloc( c4rt_vaddr_region_t *region, size_t size );
void c4rt_vaddr_free( c4rt_vaddr_region_t *region, void *ptr );
size_t c4rt_vaddr_size(c4rt_vaddr_region_t *region, void *ptr);

#endif
