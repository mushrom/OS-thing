#ifndef _kernel_memmap_c
#define _kernel_memmap_c
#include <memmap.h>

memmap_t *memmap_create( unsigned long start, unsigned long end, 
		unsigned char permissions, unsigned char present ){

	memmap_t *ret = (memmap_t *)kmalloc( sizeof( memmap_t ), 0, 0 );

	ret->start = start;
	ret->end = end;
	ret->permissions = permissions;
	ret->present = present;
	ret->references = 1;
	ret->next = 0;

	return ret;
}

void memmap_delete( memmap_t *map ){
	kfree( map );
}

// Check to see if an address is present in the address's space
int memmap_check( memmap_t *map, unsigned long address ){
	int ret = 0;

	if ( address >= map->start && address <= map->end )
		ret = 1;

	return ret;
}

memmap_t *memmaps_check( memmap_t *map, unsigned long address ){
	memmap_t *move = map;

	while( move ){
		if ( memmap_check( move, address ))
			return move;

		move = move->next;
	}

	return 0;
}

#endif
