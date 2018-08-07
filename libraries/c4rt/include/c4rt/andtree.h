#ifndef _C4RT_ANDTREE_H
#define _C4RT_ANDTREE_H 1
#include <stdbool.h>

typedef struct c4rt_andnode c4rt_andnode_t;
typedef struct c4rt_andtree c4rt_andtree_t;

typedef int (*c4rt_andtree_get_key_t)(void *b);

struct c4rt_andnode {
	c4rt_andnode_t *parent;
	c4rt_andnode_t *leaves[2];
	unsigned level;
	void *data;
};

struct c4rt_andtree {
	c4rt_andnode_t *root;
	unsigned nodes;
	c4rt_andtree_get_key_t get_key;
};

c4rt_andnode_t *andtree_insert( c4rt_andtree_t *tree, void *data );

void andtree_remove( c4rt_andtree_t *tree, c4rt_andnode_t *node );
void *andtree_remove_key( c4rt_andtree_t *tree, int key );
void *andtree_remove_data( c4rt_andtree_t *tree, void *data );

c4rt_andtree_t *andtree_init( c4rt_andtree_t *tree,
                            c4rt_andtree_get_key_t compare );
c4rt_andtree_t *andtree_deinit( c4rt_andtree_t *tree );
void andtree_print( c4rt_andtree_t *tree );
bool andtree_check( c4rt_andtree_t *tree );

c4rt_andnode_t *andtree_find_at_least( c4rt_andtree_t *tree, int key );
c4rt_andnode_t *andtree_find_data( c4rt_andtree_t *tree, void *data );
c4rt_andnode_t *andtree_find_key( c4rt_andtree_t *tree, int key );

c4rt_andnode_t *andtree_start(c4rt_andtree_t *tree);
c4rt_andnode_t *andtree_end(c4rt_andtree_t *tree);
c4rt_andnode_t *andtree_next(c4rt_andnode_t *node);
c4rt_andnode_t *andtree_previous(c4rt_andnode_t *node);

#endif
