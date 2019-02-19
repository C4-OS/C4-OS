#include <c4rt/andtree.h>
#include <c4rt/c4rt.h>
#include <stdlib.h>
#include <string.h>

/*
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <stdio.h>
*/

static c4rt_andnode_t sentinel = {
	.level = 0,
	.leaves = { &sentinel, &sentinel, },
	.parent = NULL,
	.data = NULL,
};

// values for indexing into leaf array in nodes, for readability
enum { LEFT, RIGHT };

static inline bool is_nil(c4rt_andnode_t *node){
	return node == NULL || node->level == 0;
}

static inline unsigned parentside( c4rt_andnode_t *node ){
	if (!node || !node->parent) {
		return RIGHT;
	}

	return node->parent->leaves[0] != node;
}


static inline unsigned direction( int n ){
	return !(n <= 0);
}

static inline
int andtree_compare( c4rt_andtree_t *tree, int key, c4rt_andnode_t *node ){
	return key - tree->get_key(node->data);
}

static inline c4rt_andnode_t *find_min_node( c4rt_andnode_t *node ){
	if (node->level == 0) {
		return NULL;
	}

    for ( ; !is_nil(node->leaves[LEFT]); node = node->leaves[LEFT] );

    return node;
}

static inline c4rt_andnode_t *find_max_node( c4rt_andnode_t *node ){
	if (is_nil(node)) {
		return NULL;
	}

    for ( ; !is_nil(node->leaves[RIGHT]); node = node->leaves[RIGHT] );

    return node;
}

c4rt_andnode_t *andtree_find_key( c4rt_andtree_t *tree, int key ){
    c4rt_andnode_t *temp = tree->root;

    while (!is_nil(temp)) {
		int diff = andtree_compare(tree, key, temp);

        if (diff == 0) {
            return temp;
        }

		temp = temp->leaves[direction(diff)];
    }

    return NULL;
}

// fallback recursive search for trees with duplicate elements
static inline
c4rt_andnode_t *andtree_find_dupe_data( c4rt_andtree_t *tree,
                                        c4rt_andnode_t *node,
                                        void *data )
{
	if (is_nil(node)) {
		return NULL;
	}

	if (node->data == data) {
		return node;
	}

	int diff = andtree_compare(tree, tree->get_key(data), node);

	if (diff == 0) {
		for (unsigned i = 0; i < 2; i++) {
			c4rt_andnode_t *a = andtree_find_dupe_data(tree,
			                                           node->leaves[i],
			                                           data);

			if (a) {
				return a;
			}
		}
	}

	return andtree_find_dupe_data(tree, node->leaves[direction(diff)], data);
}

c4rt_andnode_t *andtree_find_data( c4rt_andtree_t *tree, void *data ){
	c4rt_andnode_t *temp = tree->root;

	while (!is_nil(temp)) {
		int diff = andtree_compare(tree, tree->get_key(data), temp);

		if (diff == 0){
			return andtree_find_dupe_data(tree, temp, data);
		}

		temp = temp->leaves[direction(diff)];
	}

	return NULL;
}

static c4rt_andnode_t *do_find_at_least( c4rt_andtree_t *tree,
                                        c4rt_andnode_t *node, 
                                        int key )
{
	if (is_nil(node)) {
		return NULL;
	}

	int diff = andtree_compare(tree, key, node);
	unsigned dir = direction(diff);

	// found the exact value we're looking for, return it
	if (diff == 0) {
		return node;
	}

	c4rt_andnode_t *temp = do_find_at_least(tree, node->leaves[dir], key);

	if (diff < 0) {
		c4rt_andnode_t *ret = (temp && andtree_compare(tree, key, temp) > diff)? temp : node;
		return ret;
	}

	return temp;
}

c4rt_andnode_t *andtree_find_at_least( c4rt_andtree_t *tree, int key ){
	return do_find_at_least(tree, tree->root, key);
}

c4rt_andnode_t *andtree_start(c4rt_andtree_t *tree){
	return find_min_node(tree->root);
}

c4rt_andnode_t *andtree_end(c4rt_andtree_t *tree){
	return find_max_node(tree->root);
}

c4rt_andnode_t *andtree_next(c4rt_andnode_t *node){
	if (!node){
		return node;
	}

	c4rt_andnode_t *right = find_min_node(node->leaves[RIGHT]);
	c4rt_andnode_t *last = node;
	c4rt_andnode_t *p = node->parent;

	// node has right nodes to check out
	if (!is_nil(right)) {
		return right;
	}

	while (!is_nil(p) && p->leaves[RIGHT] == last){
		last = p, p = p->parent;
	}

	return p;
}

c4rt_andnode_t *andtree_previous(c4rt_andnode_t *node){
	if (!node){
		return node;
	}

	c4rt_andnode_t *left = find_max_node(node->leaves[LEFT]);
	c4rt_andnode_t *last = node;
	c4rt_andnode_t *p = node->parent;

	if (!is_nil(left)) {
		return left;
	}

	while (!is_nil(p) && p->leaves[LEFT] == last){
		last = p, p = p->parent;
	}

	return p;
}

static inline void replace_in_parent( c4rt_andnode_t *node,
                                      c4rt_andnode_t *value )
{
	if (node->parent) {
		node->parent->leaves[parentside(node)] = value;
	}

	if (!is_nil(value)) {
		value->parent = node->parent;
	}
}

static inline
c4rt_andnode_t *rotate( c4rt_andtree_t *tree, c4rt_andnode_t *node, unsigned dir ){
	c4rt_andnode_t *temp = node->leaves[!dir];

	node->leaves[!dir] = temp->leaves[dir];
	temp->leaves[dir] = node;

	if (node->leaves[!dir]){
		node->leaves[!dir]->parent = node;
	}

	replace_in_parent(node, temp);
	node->parent = temp;

    if (node == tree->root){
        tree->root = temp;
    }

	return temp;
}

static void do_node_print( c4rt_andtree_t *tree,
                           c4rt_andnode_t *node,
                           unsigned indent );


static inline
c4rt_andnode_t *andtree_skew(c4rt_andtree_t *tree, c4rt_andnode_t *node){
	c4rt_andnode_t *ret = node;

	if (is_nil(node)) {
		return ret;
	}

	if (node->leaves[LEFT]->level == node->level) {
		ret = rotate(tree, node, RIGHT);
		andtree_skew(tree, ret->leaves[RIGHT]);
	}

	return ret;
}

static inline
c4rt_andnode_t *andtree_split(c4rt_andtree_t *tree, c4rt_andnode_t *node){
	c4rt_andnode_t *ret = node;

	if (is_nil(node)) {
		return node;
	}

	if (node->level == node->leaves[RIGHT]->leaves[RIGHT]->level) {
		ret = rotate(tree, node, LEFT);
		ret->level += 1;
		andtree_split(tree, ret->leaves[RIGHT]);
	}

	return ret;
}

static inline void do_remove_node( c4rt_andtree_t *tree,
                                   c4rt_andnode_t *node,
                                   c4rt_andnode_t *replacement )
{
	// TODO: assert()
	//assert(tree->nodes > 0);

	if (node == tree->root) {
		tree->root = replacement;
	}

	replace_in_parent(node, replacement);
	c4rt_free(node);
	tree->nodes--;

}

static inline unsigned count_nodes( c4rt_andnode_t *node ){
	if (node && node->level != 0) {
		return count_nodes(node->leaves[LEFT])
		     + count_nodes(node->leaves[RIGHT])
		     + 1;
	}

	else return 0;
}

void andtree_bin_insert( c4rt_andtree_t *tree, c4rt_andnode_t *node ){
    //assert(!is_nil(node));

	if (is_nil(tree->root)) {
		tree->root = node;
		return;
	}

	c4rt_andnode_t *temp = tree->root;

	while (temp != node) {
		int diff = andtree_compare(tree, tree->get_key(node->data), temp);
		unsigned dir = !(diff < 0);

		if (is_nil(temp->leaves[dir])) {
			temp->leaves[dir] = node;
			node->parent = temp;
		}

		temp = temp->leaves[dir];
	}
}

static inline
void andtree_insert_repair( c4rt_andtree_t *tree, c4rt_andnode_t *node ){
	c4rt_andnode_t *temp = node;

	while (temp) {
		temp = andtree_skew(tree, temp);
		temp = andtree_split(tree, temp);
		temp = temp->parent;
	}
}

static inline
void andtree_remove_repair( c4rt_andtree_t *tree, c4rt_andnode_t *node ){
	c4rt_andnode_t *successor = &sentinel;
	c4rt_andnode_t *repair = NULL;

	//assert(node != NULL);

	successor = find_max_node(node->leaves[LEFT]);
	successor = successor? successor : find_min_node(node->leaves[RIGHT]);

	{
		c4rt_andnode_t *temp = (!is_nil(successor))? successor : node;

		repair = temp->parent;

		if (successor) {
			node->data = successor->data;
			do_remove_node(tree, successor,
			               successor->leaves[is_nil(successor->leaves[LEFT])]);
		}

		else {
			do_remove_node(tree, node, &sentinel);
		}
	}

	for (; !is_nil(repair); repair = repair->parent) {
		if ((repair->leaves[LEFT]->level < repair->level - 1)
		 || (repair->leaves[RIGHT]->level < repair->level - 1))
		{
			repair->level -= 1;

			if (repair->leaves[RIGHT]->level > repair->level){
				repair->leaves[RIGHT]->level = repair->level;
			}

			repair = andtree_skew(tree, repair);
			repair = andtree_split(tree, repair);

			// XXX: rarely a split will end up with too many right
			//      horizonal links in the left sub-tree, which when fixing
			//      may result in a left horizonal link which needs to be
			//      skewed...
			//
			//      can't quite figure out what I'm doing that's different
			//      from other implementations that makes this necessary, but
			//      it works, it's 5AM, my brain not work so good and
			//      it's still O(log n) so I'll leave it for now
			andtree_split(tree, repair->leaves[LEFT]);
			repair = andtree_skew(tree, repair);
		}
	}
}


static inline c4rt_andnode_t *make_andnode( void *data ){
	c4rt_andnode_t *ret = c4rt_calloc(1, sizeof(c4rt_andnode_t));

	ret->data = data;
	ret->level = 1;
	ret->leaves[LEFT] = ret->leaves[RIGHT] = &sentinel;

	return ret;
}

bool andtree_check( c4rt_andtree_t *tree );

c4rt_andnode_t *andtree_insert( c4rt_andtree_t *tree, void *data ){
	c4rt_andnode_t *node = make_andnode( data );
	//assert(node != NULL);

	andtree_bin_insert(tree, node);
	andtree_insert_repair(tree, node);
	tree->nodes += 1;

// flag to enable debugging checks, should normally be disabled
#ifdef ANDTREE_CHECK_OPS
	printf("==> should have %u nodes, have %u\n",
		   tree->nodes, count_nodes(tree->root));

	assert(tree->nodes == count_nodes(tree->root));
	assert(andtree_check(tree) == true);
#endif

	return node;
}

void andtree_remove( c4rt_andtree_t *tree, c4rt_andnode_t *node ){
	andtree_remove_repair(tree, node);

// flag to enable debugging checks, should normally be disabled
#ifdef ANDTREE_CHECK_OPS
	printf("==> should have %u nodes, have %u\n",
		   tree->nodes, count_nodes(tree->root));
	assert(tree->nodes == count_nodes(tree->root));

	if (!andtree_check(tree)){
		puts("----");
		andtree_print(tree);
		puts("----");
	}

	assert(andtree_check(tree) == true);
#endif
}

void *andtree_remove_key( c4rt_andtree_t *tree, int key ){
	c4rt_andnode_t *node = andtree_find_key(tree, key);
	void *ret = NULL;

	if (node) {
		ret = node->data;
		andtree_remove(tree, node);
	}

	return ret;
}

void *andtree_remove_data( c4rt_andtree_t *tree, void *data ){
	c4rt_andnode_t *node = andtree_find_data(tree, data);
	void *ret = NULL;

	if (node) {
		ret = node->data;
		andtree_remove(tree, node);
	}

	return ret;
}

c4rt_andtree_t *andtree_init( c4rt_andtree_t *tree,
                            c4rt_andtree_get_key_t get_key )
{
    memset(tree, 0, sizeof(*tree));
    tree->get_key = get_key;
	tree->root = &sentinel;

	return tree;
}

c4rt_andtree_t *andtree_deinit( c4rt_andtree_t *tree ){
    // TODO: fill this in
	return tree;
}

static void do_node_print( c4rt_andtree_t *tree,
                           c4rt_andnode_t *node,
                           unsigned indent )
{

	if (node->level != 0) {
		do_node_print(tree, node->leaves[0], indent + 1);

		for ( unsigned i = 0; i < indent; i++ ){
			c4_debug_printf("  |");
		}

		c4_debug_printf("- (%u) %p : %p : %d", node->level, node, node->data,
		                                       tree->get_key(node->data));

		c4_debug_printf("\n");

		do_node_print(tree, node->leaves[1], indent + 1);
	}
}

void andtree_print( c4rt_andtree_t *tree ){
	do_node_print(tree, tree->root, 0);
}

static bool check_tree( c4rt_andtree_t *tree, c4rt_andnode_t *node ){
	if (is_nil(node)) {
		return true;
	}

	bool left = check_tree(tree, node->leaves[LEFT]);
	bool right = check_tree(tree, node->leaves[RIGHT]);
	bool valid = left && right;

	if (node->level > 1) {
		if (is_nil(node->leaves[LEFT]) && is_nil(node->leaves[RIGHT])) {
			c4_debug_printf(">>> external node with level > 1! (%p)\n", node);
			valid = false;
		}

		for (unsigned i = 0; i < 2; i++) {
			if (is_nil(node->leaves[i])) {
				continue;
			}

			if (node->leaves[i]->level < node->level - 1) {
				c4_debug_printf(">>> missing pseudonode! (%p, direction: %u)\n", node, i);
				valid = false;
			}

			if (node->level < node->leaves[i]->level) {
				c4_debug_printf(">>> invalid levels! (%p, direction: %u)\n", node, i);
				valid = false;
			}
		}
	}

	if (node->leaves[LEFT]->level == node->level) {
		c4_debug_printf(">>> horizontal left link! (%p, direction)\n", node);
		valid = false;
	}

	if (node->leaves[RIGHT]->leaves[RIGHT]->level == node->level) {
		c4_debug_printf(">>> too many horizontal right links! (%p, direction)\n", node);
		valid = false;
	}

	return valid;
}

bool andtree_check( c4rt_andtree_t *tree ){
	return check_tree(tree, tree->root);
}
