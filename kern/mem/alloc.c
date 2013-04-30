/* Quick notes about the allocator:
 *
 *  - Uses binary trees to find a free space. This is just a quick and dirty
 *    allocator, and should be replaced in the future.
 *
 *  - The memory used for every allocation is the closest >= power of 2 to the
 *    size, plus 4 bytes for the leaf. Keep this in mind if allocating powers
 *    of 2 and you don't need all of it, allocating 16 bytes will allocate a
 *    total of 32 bytes of memory.
 *
 *  - It can only double in size when resizing the tree.
 *    This is because when resizing, the previous root becomes a leaf of the
 *    new root, and the leaves must be half of the total size of the parent.
 *
 *  - Because of this, the maximum allocation size without resizing is half
 *    the memory of the root. This shouldn't be a problem. 
 *
 *  - The minimum allocation size is the size of a leaf, which atm is 4 bytes.
 *    This means the smallest allocation will take up 8 bytes of heap.
 *
 *  - Because it scales by powers of 2, aligning to page boundaries is easy.
 *
 */

#ifndef _kernel_alloc_c
#define _kernel_alloc_c
#include <alloc.h>

extern void *end;
unsigned long placement = (unsigned long)&end;
unsigned long fplacement = (unsigned long)KHEAP_START; heap_t *kheap = 0;
extern page_dir_t *current_dir, *kernel_dir;
unsigned long memused = 0;

unsigned long kmalloc( unsigned long size, unsigned int align, unsigned long *physical ){ DEBUG_HERE
	if ( kheap && !kheap->full )
		return kmalloc_f( size, align, physical );
	else
		return kmalloc_e( size, align, physical );
}

/** Early kernel malloc function, allocates physical memory.	
 *  Should not be used after paging is enabled, and should be used sparingly,
 *  as there's no way to free the memory.	*/
unsigned long kmalloc_e( unsigned long size, unsigned int align, unsigned long *physical ){ DEBUG_HERE
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

unsigned long find_free_node( kmem_node_t *node, unsigned long node_size, unsigned long size, unsigned long align ){
	unsigned long ret = 0;
	kmem_node_t *temp = node;
	int s = node_size / 2;
	temp += s;

	//printf( "entered new ff: node=0x%x temp=0x%x\n", node, temp );
	if ( s <= KHEAP_MIN_ALLOC || ( temp->magics == KHEAP_MAGIC && node->magics == KHEAP_MAGIC ))
		return 0;

	if ( size < s / 2 ){
		if ((	ret = find_free_node( temp, s, size, align )) ||
		    (	ret = find_free_node( node, s, size, align )))
			return ret;
	}

	if ( temp->magics != KHEAP_MAGIC ){
		temp->magics = KHEAP_MAGIC;
		return (unsigned long)temp + sizeof( kmem_node_t );
	} else if ( node->magics != KHEAP_MAGIC ){
		node->magics = KHEAP_MAGIC;
		return (unsigned long)node + sizeof( kmem_node_t );
	}
	
	return 0;
}

/* Final malloc function, should be used after paging is enabled. */
unsigned long malloc_f( heap_t *heap, unsigned long size, unsigned int align, unsigned long *physical ){ DEBUG_HERE
	unsigned long ret = 0, r_count = 0;
	memused += size + sizeof( kmem_node_t );

	ret = find_free_node((kmem_node_t *)heap->root, heap->size, size + sizeof( kmem_node_t ), align );
	while ( ret == 0 ){
		expand( heap );
		ret = find_free_node((kmem_node_t *)heap->root, heap->size, size + sizeof( kmem_node_t ), align );

		if ( r_count++ > 3 )
			PANIC( "well feck" );
		
	}

	//printf( "ret: 0x%x\n", ret );
	if ( physical )
		*physical = get_page( ret, current_dir );
	
	return (unsigned long)ret;
}

unsigned long kmalloc_f( unsigned long size, unsigned int align, unsigned long *physical ){ DEBUG_HERE
	return malloc_f( kheap, size, align, physical );
}

void free_f( heap_t *heap, void *ptr ){
	kmem_node_t *p = ptr - sizeof( kmem_node_t );
	if ( heap ){
		if ( p->magics == KHEAP_MAGIC ){
			p->magics = KHEAP_FREED;
		} else {
			printf( "Could not free pointer 0x%x.\n", ptr );
			if ( p->magics == KHEAP_FREED )
				printf( "Likely a double-freed pointer.\n" );
			PANIC( "bad free\n" );
		}
	}
}

void kfree( void *ptr ){
	free_f( kheap, ptr );
}

unsigned long get_memused( void ){
	return memused;
}

/** Note: This assumes the memory needed is already mapped to pages. */
struct heap *init_heap( unsigned long start, unsigned long size ){ DEBUG_HERE
	heap_t *heap = (void *)kmalloc( sizeof( heap_t ), 0, 0 );
	memset( heap, 0, sizeof( heap_t ));
	heap->magics = KHEAP_MAGIC;
	heap->size   = size / 4;
	heap->root   = (void *)start;
	heap->full   = 0;
	return (struct heap *)heap;
}

int r_check = 0;
void expand( heap_t *heap ){
	unsigned long old_size = heap->size * 4;
	unsigned long new_size = old_size * 2;
	unsigned long root = (unsigned long)heap->root;
	heap->full = 1;

	printf( "============================\n" );
	printf( "Attempting to resize heap...\n" );
	printf( "\told = 0x%x 0x%x\n", old_size, root + old_size );
	printf( "\tnew = 0x%x 0x%x\n", new_size, root + new_size );
	printf( "============================\n"  );

	map_pages( root + old_size, root + new_size + 0x100 + 0x10000, 7, current_dir );
	flush_tlb( );

	if ( r_check++ )
		PANIC( "Caught another expand" );

	heap->size = new_size/4;
	heap->full = 0;

	//PANIC( "Expand not implemented yet." );
}

#endif
