#ifndef _C4ALLOC_H
#define _C4ALLOC_H 1
#include <stdint.h>

enum {
	// maximum allocation size will be limited by the number of buckets,
	// since each index will be log2(size) of the blocks it stores
	C4A_MAX_BUCKETS = 12,
};

enum {
	C4A_STATUS_FREE  = 1,
	C4A_STATUS_USED  = 2,
};

typedef struct c4a_node {
	struct c4a_node *previous;
	struct c4a_node *next;
	unsigned status;
	unsigned bucket;
} c4a_node_t;

typedef struct c4a_bucket {
	c4a_node_t *start;
} c4a_bucket_t;

typedef struct c4a_heap {
	uintptr_t start;
	unsigned  pages;

	c4a_bucket_t buckets[C4A_MAX_BUCKETS + 1];
} c4a_heap_t;

void *c4a_alloc( c4a_heap_t *heap, unsigned size );
void  c4a_free( c4a_heap_t *heap, void *ptr );

void c4a_heap_init( c4a_heap_t *heap, uintptr_t location );
void c4a_heap_deinit( c4a_heap_t *heap );

#endif
