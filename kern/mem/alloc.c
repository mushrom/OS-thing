#ifndef _kernel_alloc_c
#define _kernel_alloc_c
#include <mem/alloc.h>

extern void *end;
unsigned long placement = (unsigned long)&end;
heap_t *kheap;
unsigned int	block_size = sizeof( kmemnode_t );

unsigned long kmalloc( unsigned long size, unsigned int align, unsigned long *physical ){ DEBUG_HERE
	return kmalloc_e( size, align, physical );
}

unsigned long kmalloc_e( unsigned long size, unsigned int align, unsigned long *physical ){ DEBUG_HERE
	/* Early kernel malloc function, allocates physical memory.	
 	 * Should not be used after paging is enabled, and should be used sparingly,
 	 * as there's no way to free the memory	*/
	unsigned long addr;
	if ( align && ( placement & 0xfff )){ DEBUG_HERE
		placement &= 0xfffff000;
		placement += 0X1000;
	}
	if ( physical ){ DEBUG_HERE
		*physical = placement;
	}
	addr = placement;
	placement += size;
	return addr;
}

unsigned long kmalloc_f( unsigned long size, unsigned int align, unsigned long *physical ){ DEBUG_HERE
	/* Final malloc function, should only be used after paging is enabled. */
	return 0xc0000000;
}

void init_heap( unsigned long start, unsigned long size, page_dir_t *dir ){ DEBUG_HERE
	unsigned long i;
	kheap = (void *)kmalloc( sizeof( heap_t ), 0, 0 );
	kheap->memmove = kheap->memptr = kheap->memroot = (void *)start;
	kmemnode_t *memptr = kheap->memptr;

	i = start;
	//printf( "    block size: %d, 0x%x\n", block_size, get_page( start, 0, dir ));

	for ( i = start; i < start + (block_size*4); i += block_size ){ DEBUG_HERE
		memptr->next = memptr + block_size;
		memptr->magics = KHEAP_MAGIC;
		printf( "Alloced block 0x%x->0x%x, %x\n", memptr, memptr->next, memptr->magics );
		printf( "    [ 0x%x->0x%x ]\n", &memptr, &memptr->next );
		memptr = &memptr->next;
	}
}

#endif
