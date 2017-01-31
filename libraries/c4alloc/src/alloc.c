#include <c4alloc/c4alloc.h>
#include <c4rt/c4rt.h>
#include <c4/paging.h>
#include <stdint.h>

static inline unsigned ulog2( unsigned n ){
	unsigned i;

	for ( i = 0; n && (n >>= 1); i++ );

	return i;
}

static inline void c4a_bucket_insert( c4a_bucket_t *bucket, c4a_node_t *node ){
	// TODO: assert node has no previous node
	node->next = bucket->start;
	bucket->start = node;
}

static inline c4a_node_t *c4a_bucket_pop( c4a_bucket_t *bucket ){
	c4a_node_t *ret = bucket->start;

	if ( ret ){
		bucket->start = bucket->start->next;
	}

	return ret;
}

static inline c4a_bucket_t *c4a_bucket( c4a_heap_t *heap, unsigned bucket ){
	return heap->buckets + bucket;
}

static inline void c4a_node_unlink( c4a_heap_t *heap, c4a_node_t *node ){
	if ( node->previous ){
		node->previous->next = node->next;
		node->previous = NULL;
	}

	if ( node->next ){
		node->next->previous = node->previous;
		node->previous = NULL;
	}

	if ( node == c4a_bucket(heap, node->bucket)->start ){
		c4a_bucket(heap, node->bucket)->start = NULL;
	}
}

static inline void c4a_grow( c4a_heap_t *heap, unsigned pages ){
	uintptr_t temp = heap->start + heap->pages * PAGE_SIZE;
	uintptr_t end  = heap->start + (heap->pages + pages) * PAGE_SIZE;
	unsigned index = ulog2( PAGE_SIZE );

	heap->pages += pages;

	for ( ; temp < end; temp += PAGE_SIZE ){
		c4a_node_t *node = (void *)temp;

		c4_request_page( c4_get_pager(), temp, PAGE_READ | PAGE_WRITE );

		node->status = C4A_STATUS_FREE;
		node->bucket = index;
		c4a_bucket_insert( c4a_bucket(heap, index), node );
	}
}

static inline unsigned c4a_find_upwards_bucket( c4a_heap_t *heap,
                                                unsigned index )
{
	for ( unsigned k = index; k <= C4A_MAX_BUCKETS; k++ ){
		if ( c4a_bucket(heap, k)->start ){
			return k;
		}
	}

	// no free nodes found in the buckets, try to allocate another page
	// and return that if it succeeds
	c4a_grow( heap, 1 );

	return ulog2( PAGE_SIZE );
}

static inline c4a_node_t *c4a_split_node( c4a_node_t *node ){
	uint8_t    *ptr  = (uint8_t *)node;
	c4a_node_t *temp = (c4a_node_t *)(ptr + (1 << (node->bucket - 1)));

	node->bucket--;

	temp->bucket   = node->bucket;
	temp->status   = node->status;
	temp->next     = node->next;
	temp->previous = node;
	node->next     = temp;

	return temp;
}

static inline void c4a_bucket_spillover( c4a_heap_t *heap,
                                         unsigned lower,
                                         unsigned upper )
{
	c4a_node_t *node = c4a_bucket_pop( c4a_bucket( heap, upper ));

	for ( unsigned i = upper; i > lower; i-- ){
		unsigned prev = i - 1;
		c4a_node_t *temp = c4a_split_node( node );

		c4a_bucket_insert( c4a_bucket(heap, prev), temp );
	}
}

c4a_node_t *c4a_get_node_from_bucket( c4a_heap_t *heap, unsigned index ){
	c4a_node_t *ret = NULL;

	ret = c4a_bucket_pop( c4a_bucket( heap, index ));

	if ( !ret ){
		unsigned next = c4a_find_upwards_bucket( heap, index + 1 );
		c4a_bucket_spillover( heap, index, next );
		ret = c4a_bucket_pop( c4a_bucket( heap, index ));
	}

	return ret;
}

void *c4a_alloc( c4a_heap_t *heap, unsigned size ){
	unsigned index = ulog2( size + sizeof( c4a_node_t ));

	// if the log2 approximation isn't equal, assume size is greater than
	// 2**index and round up to the next power of 2
	if ( size != (1 << index) ){
		index++;
	}

	if ( index > C4A_MAX_BUCKETS ){
		return NULL;
	}

	c4a_node_t *node = c4a_get_node_from_bucket( heap, index );
	node->status = C4A_STATUS_USED;

	return node + 1;
}

void c4a_free( c4a_heap_t *heap, void *ptr ){
	c4a_node_t *node = (c4a_node_t *)ptr - 1;

	node->status = C4A_STATUS_FREE;
	c4a_bucket_insert( c4a_bucket( heap, node->bucket ), node );
}

void c4a_heap_init( c4a_heap_t *heap, uintptr_t location ){
	uint8_t *ptr = (uint8_t *)heap;

	// TODO: memset()
	for ( unsigned i = 0; i < sizeof( c4a_heap_t ); i++ ){
		ptr[i] = 0;
	}

	heap->start = location;
}

void c4a_heap_deinit( c4a_heap_t *heap ){
	// nothing here atm
	;
}
