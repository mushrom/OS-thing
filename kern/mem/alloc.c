#ifndef _kernel_alloc_c
#define _kernel_alloc_c
#include <mem/alloc.h>

extern void *end;
unsigned long placement = (unsigned long)&end;
unsigned long fplacement = (unsigned long)KHEAP_START; heap_t *kheap = 0;
unsigned int	block_size = sizeof( kmemnode_t );
extern page_dir_t *current_dir, *kernel_dir;
unsigned long memused = 0;

unsigned long kmalloc( unsigned long size, unsigned int align, unsigned long *physical ){ DEBUG_HERE
	if ( kheap )
		return kmalloc_f( size, align, physical );
	else
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
	int addr;
	if ( align && ( fplacement & 0xfff )){
		fplacement &= 0xfffff000;
		fplacement += 0x1000;
	}
	if ( physical )
		*physical = get_page( fplacement, 0, kernel_dir );

	addr = fplacement;
	fplacement += size;
	memused += size;
	return addr;
	/*
	unsigned int found_free = 0, i = 0;
	kmemnode_t *memptr = kheap->memptr;
	kmemnode_t *memmove = memptr;
	kmemnode_t *ret;
	memptr = kheap->memroot;
	//printf( "[mem] 1:" );
	while ( !found_free ){
		if ( memptr->size == 0 ){
			memmove = memptr;
			for ( i = 0; i < size; ){
				if ( memmove->next->size == 0 ){
					i += block_size;
					memmove = memmove->next;
				} else {
					i = 0;
					break;
				}
			}
			if ( i ){
				ret = memptr->next;
				memptr->next = memmove->next;
				memptr->next->prev = memptr;
				memptr->size = i;
				found_free = 1;
				break;
			}
		} else {
			memptr = memptr->next;
		}
	}
	memused += i + block_size;
	//printf( "[mem] 2 " );
	return (unsigned long)ret;
	*/
}

void kfree( void *ptr ){
	kmemnode_t *memptr  = ptr;
	kmemnode_t *memmove = ptr;
	printf( "%s: ", memmove );

	memmove = memptr - block_size;
	if ( memmove->magics == KHEAP_MAGIC && memmove->size != 0 )
		printf( "Can free block %d, magic: 0x%x, size: 0x%x bytes: %s\n", 
			memmove->next - memmove, memmove->magics, memmove->size, (char *)memmove + block_size );
	else 
		printf( "Corrupted block at 0x%x->0x%x! (magic: 0x%x, size 0x%x)\n", 
			memmove, memmove->next, memmove->magics, memmove->size );
}

unsigned long get_memused( void ){
	return memused;
}

struct heap *init_heap( unsigned long start, unsigned long size, page_dir_t *dir ){ DEBUG_HERE
	unsigned long block_c = 0, i = 0;
	heap_t *heap = (void *)kmalloc( sizeof( heap_t ), 0, 0 );
	heap->memmove = heap->memptr = heap->memroot = (void *)start;
	kmemnode_t *memptr = heap->memptr;

	for ( memptr = (void *)start; (unsigned long)memptr < start + size; block_c++ ){ DEBUG_HERE
	//for ( i = start; i < start + size; i += block_size, block_c++ ){ DEBUG_HERE
		/*
		memptr = i;
 		memptr->next = i + block_size;
		memptr->prev = i - block_size;
		*/
 		memptr->next = memptr + block_size;
		memptr->prev = memptr - block_size;
		memptr->size = 0;
		memptr->magics = KHEAP_MAGIC;
		//printf( "Alloced block %d, 0x%x->0x%x, %x\n", memptr->next-memptr, memptr, memptr->next, memptr->magics );
		memptr = memptr->next;
	}
	printf( "    initialised heap, 0x%x, block size: %d bytes, %d blocks\n", heap->memroot, block_size, block_c );
	return (struct heap *)heap;
}

#endif
