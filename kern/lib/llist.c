#ifndef _kernel_lib_llist_c
#define _kernel_lib_llist_c
#include <llist.h>

llist_node_t *l_add_node_end( llist_node_t *node, void *data ){
	llist_node_t *move;

	if ( node ){
		for ( move = node; move->next; move = move->next );
		move->next = knew( node );
		move->next->prev = move;
		move->data = data;
		return move->next;
	} else {
		return knew( node );
	}

	return 0;
}

llist_node_t *l_add_node_begin( llist_node_t *node, void *data ){
	llist_node_t *move;

	if ( node ){
		for ( move = node; move->prev; move = move->prev );
		move->prev = knew( node );
		move->prev->next = move;
		move->data = data;
	} else {
		return knew( node );
	}

	return 0;
}

/*
void *l_remove_node( llist_node_t *node );
void *l_remove_all_nodes( llist_node_t *node );
*/

#endif
