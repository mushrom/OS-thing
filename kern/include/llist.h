#ifndef _kernel_lib_llist_h
#define _kernel_lib_llist_h
#include <kmacros.h>
#include <alloc.h>
#include <string.h>

typedef struct llist_node {
	struct llist_node *next;
	struct llist_node *prev;

	void *data;
	int val;
} llist_node_t;

llist_node_t *l_add_node_end( llist_node_t *node, void *data );
llist_node_t *l_add_node_begin( llist_node_t *node, void *data );
llist_node_t *l_get_node_index( llist_node_t *node, int index );

void l_remove_node( llist_node_t *node );
void l_remove_all_nodes( llist_node_t *node );

#endif
