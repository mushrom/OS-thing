#ifndef _kernel_alloc_c
#define _kernel_alloc_c
#include <mem/alloc.h>

extern void *end;
unsigned long placement = (unsigned long)&end;
heap_t *kheap;
unsigned int	block_size = sizeof( kmemnode_t );

unsigned long kmalloc( unsigned long size, unsigned int align, unsigned long *physical ){
	return kmalloc_e( size, align, physical );
}

unsigned long kmalloc_e( unsigned long size, unsigned int align, unsigned long *physical ){
	/* Early kernel malloc function, allocates physical memory.
 	 * Should not be used after paging is enabled, obviously.   */
	unsigned long addr;
	if ( align && ( placement & 0xfff )){
		placement &= 0xfffff000;
		placement += 0X1000;
	}
	if ( physical ){
		*physical = placement;
	}
	addr = placement;
	placement += size;
	return addr;
}

unsigned long kmalloc_f( unsigned long size, unsigned int align, unsigned long *physical ){
	/* Final malloc function, should only be used after paging is enabled. */
	return 0xc0000000;
}

void init_heap( unsigned long start, unsigned long size, page_dir_t *dir ){
	unsigned long i, addr;
	kheap = (void *)kmalloc( 16, 0, 0 );
	kheap->kmemmove = kheap->kmemptr = kheap->kmemroot = (void *)start;

	for ( addr = start; addr < start + size; addr += PAGE_SIZE ){
		alloc_page( get_page( addr, 1, dir ));
		//wait( 1 );
	}
	for ( addr = start; addr < start + size; addr += PAGE_SIZE ){
		set_table_perms( PAGE_USER | PAGE_WRITEABLE | PAGE_PRESENT, addr, dir );
	}

	i = start;
	/*
	printf( "    block size: %d\n", block_size );

	for ( i = start; i < start + size; i += block_size ){
		kheap->kmemptr->next = kheap->kmemptr + block_size;
		kheap->kmemptr->prev = kheap->kmemptr - block_size;
		kheap->kmemptr->size = 0;
		kheap->kmemptr->magics = KHEAP_MAGIC;

		kheap->kmemptr = kheap->kmemptr->next;
	}
	*/
}

#endif
