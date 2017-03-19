#include <c4alloc/c4alloc.h>
#include <interfaces/pager.h>
#include <c4rt/c4rt.h>
#include <c4/paging.h>
#include <stdint.h>

#ifdef C4ALLOC_DEBUG
#define DBG_PRINT( format, ... ) \
	c4_debug_printf( "--- c4alloc: " format, __VA_ARGS__ )
#else
#define DBG_PRINT( format, ... ) /* debug: format, __VAR_ARGS__ ) */
#endif

static inline unsigned ulog2( unsigned n ){
	unsigned i;

	for ( i = 0; n && (n >>= 1); i++ );

	return i;
}

static inline c4a_bucket_t *c4a_bucket( c4a_heap_t *heap, unsigned bucket ){
	return heap->buckets + bucket;
}

static inline void c4a_node_unlink( c4a_heap_t *heap, c4a_node_t *node ){
	if ( node == c4a_bucket(heap, node->bucket)->start ){
		c4a_bucket(heap, node->bucket)->start = node->next;
	}

	if ( node->previous ){
		node->previous->next = node->next;
	}

	if ( node->next ){
		node->next->previous = node->previous;
	}

	node->previous = NULL;
	node->next = NULL;
}

static inline void c4a_bucket_insert( c4a_bucket_t *bucket, c4a_node_t *node ){
	// TODO: assert node has no previous node
	if ( bucket->start ){
		bucket->start->previous = node;
	}

	node->next     = bucket->start;
	node->previous = NULL;
	bucket->start  = node;
}

static inline c4a_node_t *c4a_bucket_pop( c4a_heap_t *heap, unsigned index ){
	c4a_bucket_t *bucket = c4a_bucket( heap, index );
	c4a_node_t *ret = bucket->start;

	if ( ret ){
		c4a_node_unlink( heap, ret );
	}

	return ret;
}

static inline void c4a_grow( c4a_heap_t *heap ){
	unsigned  pages = (1 << C4A_MAX_BUCKETS) / PAGE_SIZE;
	uintptr_t temp  = heap->start + heap->pages * PAGE_SIZE;
	uintptr_t end   = heap->start + (heap->pages + pages) * PAGE_SIZE;
	unsigned  index = C4A_MAX_BUCKETS;

	heap->pages += pages;

	for ( ; temp < end; temp += pages * PAGE_SIZE ){
		c4a_node_t *node = (void *)temp;

		pager_request_pages( c4_get_pager(), temp,
		                     PAGE_READ | PAGE_WRITE, pages );

		node->status = C4A_STATUS_FREE;
		node->bucket = index;
		node->phys_next = node->phys_prev = node->next = node->previous = NULL;
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
	c4a_grow( heap );

	return C4A_MAX_BUCKETS;
}

static inline c4a_node_t *c4a_split_node( c4a_node_t *node ){
	uint8_t    *ptr  = (uint8_t *)node;
	c4a_node_t *temp = (c4a_node_t *)(ptr + (1 << (node->bucket - 1)));

	DBG_PRINT( "splitting %p into %p and %p\n", node, node, temp );

	node->bucket--;

	temp->bucket   = node->bucket;
	temp->status   = node->status;
	temp->next     = node->next;
	temp->previous = node;
	node->next     = temp;

	temp->phys_prev = node;
	temp->phys_next = node->phys_next;
	node->phys_next = temp;

	if ( temp->phys_next ){
		temp->phys_next->phys_prev = temp;
	}

	return temp;
}

static inline void c4a_bucket_spillover( c4a_heap_t *heap,
                                         unsigned lower,
                                         unsigned upper )
{
	c4a_node_t *node = c4a_bucket_pop( heap, upper );
	DBG_PRINT( "spilling bucket %u down to %u\n", upper, lower );

	for ( unsigned i = upper; i > lower; i-- ){
		unsigned prev = i - 1;
		c4a_node_t *temp = c4a_split_node( node );

		c4a_bucket_insert( c4a_bucket(heap, prev), temp );
	}

	c4a_bucket_insert( c4a_bucket(heap, lower), node );
}

c4a_node_t *c4a_get_node_from_bucket( c4a_heap_t *heap, unsigned index ){
	c4a_node_t *ret = NULL;

	ret = c4a_bucket_pop( heap, index );

	if ( !ret ){
		unsigned next = c4a_find_upwards_bucket( heap, index + 1 );
		c4a_bucket_spillover( heap, index, next );
		ret = c4a_bucket_pop( heap, index );
		c4a_node_unlink( heap, ret );
	}

	return ret;
}

void *c4a_alloc( c4a_heap_t *heap, unsigned size ){
	unsigned adjusted = size + sizeof( c4a_node_t );
	unsigned index    = ulog2( adjusted );

	// if the log2 approximation isn't equal, assume size is greater than
	// 2**index and round up to the next power of 2
	if ( adjusted != (1 << index) ){
		index++;
	}

	if ( index > C4A_MAX_BUCKETS ){
		return NULL;
	}

	c4a_node_t *node = c4a_get_node_from_bucket( heap, index );
	node->status = C4A_STATUS_USED;

	DBG_PRINT( "allocating %u (%u) from bucket %u (%u)\n",
		size, adjusted, index, (1 << index) );

	return node + 1;
}

static inline void c4a_dump_buckets( c4a_heap_t *heap ){
	for ( unsigned i = 0; i <= C4A_MAX_BUCKETS; i++ ){
		c4a_node_t *temp = heap->buckets[i].start;

		DBG_PRINT( "bucket[%u]: ", i );

		while ( temp ){
			c4_debug_printf( "-> %p ", temp );

			if ( temp == temp->next ){
				c4_debug_printf( "(ERROR: cycle)" );
				break;
			}

			temp = temp->next;
		}

		c4_debug_printf( "\n" );
	}
}

static inline bool c4a_can_coalesce( c4a_node_t *node, c4a_node_t *next ){
	return node         != NULL
	    && next         != NULL
	    && node->status == C4A_STATUS_FREE
	    && next->status == C4A_STATUS_FREE
	    && node->bucket == next->bucket; 
}

static inline bool c4a_do_coalesce( c4a_heap_t *heap,
                                    c4a_node_t *a,
                                    c4a_node_t *b )
{
	if ( c4a_can_coalesce( a, b )){
		c4a_node_unlink( heap, a );
		c4a_node_unlink( heap, b );

		a->bucket++;
		a->phys_next = b->phys_next;

		if ( a->phys_next ){
			a->phys_next->phys_prev = a;
		}

		return true;
	}

	return false;
}

static inline c4a_node_t *c4a_coalesce( c4a_heap_t *heap, c4a_node_t *node ){
	bool coalesced = false;

	do {
		c4a_node_t *temp = node->phys_prev;

		coalesced  = c4a_do_coalesce( heap, node->phys_prev, node );
		node = coalesced? temp : node;
		coalesced |= c4a_do_coalesce( heap, node, node->phys_next );

	} while ( coalesced );

	return node;
}

void c4a_free( c4a_heap_t *heap, void *ptr ){
	c4a_node_t *node = (c4a_node_t *)ptr - 1;

	if ( node->status != C4A_STATUS_USED ){
		c4_debug_printf(
			"--- c4alloc: Have double free or corrupted block!\n"
			"--- c4alloc: - at %p from %p, cowardly refusing to continue...\n"
			"--- c4alloc: - current thread: %u, pager: %u\n",
			node, ptr, c4_get_id(), c4_get_pager() );

		return;
	}

	node->status   = C4A_STATUS_FREE;
	node->next     = NULL;
	node->previous = NULL;

	node = c4a_coalesce( heap, node );
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
