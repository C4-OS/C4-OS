#include <c4rt/addrman.h>
#include <c4rt/andtree.h>
#include <c4rt/stublibc.h>

#define PAGE_SIZE 0x1000

static int vnode_get_offset(void *a){
	c4rt_vnode_t *node_a = a;

	return (int)node_a->offset;
}

static int vnode_get_size(void *a){
	c4rt_vnode_t *node_a = a;

	return (int)node_a->size;
}

static inline c4rt_vnode_t *make_vnode( unsigned size, unsigned offset ){
	c4rt_vnode_t *ret = calloc(1, sizeof(c4rt_vnode_t));

	*ret = (c4rt_vnode_t){
		.size       = size,
		.offset     = offset,
		.is_free    = true,
	};

	return ret;
}

static inline c4rt_vnode_t *split_vnode( c4rt_vaddr_region_t *region,
                                         c4rt_vnode_t *node,
                                         unsigned size )
{
	// TODO: implement assert
	/*
	assert(node != NULL);
	assert(size <=  node->size);
	assert(size != 0);

	assert(andtree_remove_data(&region->free_nodes, node) != NULL);
	*/

	andtree_remove_data(&region->free_nodes, node);
	node->is_free = false;

	if (size != node->size) {
		andtree_remove_data(&region->layout, node);

		c4rt_vnode_t *vn = make_vnode(node->size - size, node->offset + size);
		node->size  = size;

		andtree_insert(&region->free_nodes, vn);
		andtree_insert(&region->layout, node);
		andtree_insert(&region->layout, vn);
	}

	return node;
}

static inline c4rt_vnode_t *try_merge( c4rt_vaddr_region_t *region,
                                       c4rt_vnode_t *a,
                                       c4rt_vnode_t *b )
{
	if (!a || !b){
		return NULL;
	}

	if (!a->is_free || !b->is_free){
		return NULL;
	}

	c4rt_vnode_t *lower = (a->offset < b->offset)? a : b;
	c4rt_vnode_t *upper = (a->offset < b->offset)? b : a;

	/*
	printf("!!! merging %p and %p to size %u\n",
	       lower, upper, lower->size + upper->size);
		   */

	// TODO: implement assert
	/*
	assert(andtree_remove_data(&region->layout, upper) != NULL);
	assert(andtree_remove_data(&region->free_nodes, lower) != NULL);
	assert(andtree_remove_data(&region->free_nodes, upper) != NULL);
	*/

	andtree_remove_data(&region->layout, upper);
	andtree_remove_data(&region->free_nodes, lower);
	andtree_remove_data(&region->free_nodes, upper);

	lower->size += upper->size;
	andtree_insert(&region->free_nodes, lower);

	return lower;
}

static inline void free_vnode( c4rt_vaddr_region_t *region,
                               c4rt_vnode_t *node )
{
	// TODO: assert()
	//assert(node != NULL);

	node->is_free = true;
	// TODO: this insertion isn't really necessary, but makes the merging
	//       process a bit simpler, maybe remove in the future if optimizing
	//       that seems fun
	andtree_insert(&region->free_nodes, node);

	c4rt_vnode_t   *temp = node;

	while (temp) {
		c4rt_andnode_t *laynode = andtree_find_data(&region->layout, temp);
		// TODO: assert()
		//assert(laynode != NULL);

		c4rt_andnode_t *prevnode = andtree_previous(laynode);
		c4rt_andnode_t *nextnode = andtree_next(laynode);

		c4rt_vnode_t *prev = prevnode? prevnode->data : NULL;
		c4rt_vnode_t *next = nextnode? nextnode->data : NULL;

		c4rt_vnode_t *foo = try_merge(region, prev, temp);
		temp = foo? foo : try_merge(region, temp, next);
	}
}

static inline c4rt_vnode_t *alloc_vnode( c4rt_vaddr_region_t *region,
                                         unsigned size )
{
	c4rt_andnode_t *temp = andtree_find_at_least(&region->free_nodes, size);

	if (!temp) {
		return NULL;
	}

	c4rt_vnode_t *node = temp->data;

	if ( size <= node->size ){
		return split_vnode(region, node, size);
	}

	return NULL;
}

c4rt_vaddr_region_t *c4rt_vaddr_region_create( uintptr_t base, unsigned pages ){
	c4rt_vaddr_region_t *ret = calloc(1, sizeof(c4rt_vaddr_region_t));
	c4rt_vnode_t *root = make_vnode(pages, 0);

	ret->base.num = base;

	andtree_init(&ret->layout,     vnode_get_offset);
	andtree_init(&ret->free_nodes, vnode_get_size);

	andtree_insert(&ret->layout, root);
	andtree_insert(&ret->free_nodes, root);

	return ret;
}

void *c4rt_vaddr_alloc( c4rt_vaddr_region_t *region, size_t size ){
	size_t pages = (size / PAGE_SIZE) + (size % PAGE_SIZE != 0);
	c4rt_vnode_t *node = alloc_vnode(region, pages);

	if (!node) {
		return NULL;
	}

	return (void *)(region->base.num + node->offset * PAGE_SIZE);
}

void c4rt_vaddr_free( c4rt_vaddr_region_t *region, void *ptr ){
	uintptr_t addr = (uintptr_t)ptr;
	unsigned offset = (addr - region->base.num) / PAGE_SIZE;
	c4rt_andnode_t *node = andtree_find_key(&region->layout, offset);

	if (!node) {
		return;
	}

	free_vnode(region, node->data);
}

size_t c4rt_vaddr_size(c4rt_vaddr_region_t *region, void *ptr){
	uintptr_t addr = (uintptr_t)ptr;
	unsigned offset = (addr - region->base.num) / PAGE_SIZE;
	c4rt_andnode_t *node = andtree_find_key(&region->layout, offset);

	if (!node) {
		return 0;
	}

	return vnode_get_size(node->data) * PAGE_SIZE;
}
