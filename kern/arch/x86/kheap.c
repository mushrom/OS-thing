#ifndef _kernel_memory_c
#define _kernel_memory_c
#include <arch/x86/kheap.h>

extern void *end;
uint32_t placement_address = (uint32_t)&end;
extern page_directory_t *kernel_directory, *current_directory;
heap_t *kheap;

void *kmalloc( uint32_t size, uint8_t align, uint32_t *phys ){
	void *tmp = 0;

	if ( kheap != 0 ){
		tmp = alloc( size, align, kheap );
		if ( phys != 0 ){
			page_t *page = get_page((uint32_t)tmp, 0, kernel_directory );
			*phys = page->frame * 0x1000 + ((uint32_t)tmp & 0xfff );
		}
		return tmp;
	} else {
		if ( align && ( placement_address & 0xfffff000 )){
			placement_address &= 0xfffff000;
			placement_address += 0x1000;
		}
		if ( phys ){
			*phys = placement_address;
		}
		tmp = (void *)placement_address;
		placement_address += size;
		return tmp;
	}
}

void kfree( void *p ){
	if ( kheap != 0 ){
		free( p, kheap );
	}
}

static int32_t find_smallest_hole( uint32_t size, uint8_t page_align, heap_t *heap ){
	header_t *header;
	uint32_t i = 0;
	uint32_t location; 
	int32_t  offset, hole_size;

	i = 0;
	while ( i < heap->index.size ){	
		header = (header_t *)lookup_ordered_array( i, &heap->index );
		if ( page_align ){
			location = (uint32_t)header;
			offset   = 0;
			if ((( location + sizeof( header_t )) & 0xfffff000 ) != 0 ){
				offset = 0x1000 - ( location + sizeof( header_t )) % 0x1000;
			}
			hole_size = (int32_t)header->size - offset;
			if ( hole_size >= (int32_t)size ){
				break;
			}
		} else if ( header->size >= size ){
			break;
		}
		i++;
	}
	if ( i == heap->index.size ){
		return -1;
	} else {
		return i;
	}
}

static uint8_t header_t_less_than( void *a, void *b ){
	return ((( header_t *)a)->size < ((header_t *)b)->size );
}

static void expand( uint32_t new_size, heap_t *heap ){
	uint32_t old_size;
	uint32_t i;
	
	assert( new_size > heap->end_addr - heap->start_addr, );
	if (( new_size & 0xfffff000 ) != 0 ){
		new_size &= 0xfffff000;
		new_size += 0x1000;
	}
	assert( heap->start_addr + new_size <= heap->max_addr, );
	i = old_size = heap->end_addr - heap->start_addr;
	for ( ; i < new_size; i += 0x1000 ){
		alloc_frame( get_page( heap->start_addr + i, 1, kernel_directory ),
				heap->supervisor, heap->readonly );
	}

	heap->end_addr = heap->start_addr + new_size;
}

static uint32_t contract( uint32_t new_size, heap_t *heap ){
	assert( new_size < heap->end_addr - heap->start_addr, 0 );
	uint32_t old_size, i;

	if ( new_size & 0x1000 ){
		new_size &= 0x1000;
		new_size += 0x1000;
	}

	if ( new_size < HEAP_MIN_SIZE ){
		new_size = HEAP_MIN_SIZE;
	}

	i = old_size = heap->end_addr - heap->start_addr;
	while ( i > new_size ){
		free_frame( get_page( heap->start_addr + i, 0, kernel_directory ));
		i -= 0x1000;
	}
	heap->end_addr = heap->start_addr + new_size;

	return new_size;
}

void *alloc( uint32_t size, uint8_t align, heap_t *heap ){
	uint32_t new_size = size + sizeof( header_t ) + sizeof( footer_t ), 
		 orig_hole_pos, 
		 orig_hole_size;
	uint32_t new_location;
	int32_t  i = find_smallest_hole( new_size, align, heap );
	if ( i < 0 ){
		uint32_t old_length = heap->end_addr - heap->start_addr;
		uint32_t old_end_addr = heap->end_addr,
			 new_length,
			 idx = -1,
			 tmp,
			 value = 0x0;
		header_t *header;
		footer_t *footer;

		expand( old_length + new_size, heap );
		new_length = heap->end_addr - heap->start_addr;

		idx = -1; value = 0x0;
		for ( i = 0; i < heap->index.size; i++ ){
			tmp = (uint32_t)lookup_ordered_array( i, &heap->index );
			if ( tmp > value ){
				value = tmp;
				idx = i;
			}
		}

		if ( idx == -1 ){
			header = (header_t *)old_end_addr;
			header->magic = HEAP_MAGIC;
			header->size = new_length - old_length;
			header->is_hole = 1;
			
			footer = (footer_t *)( old_end_addr + header->size - sizeof( footer_t ));
			footer->magic = HEAP_MAGIC;
			footer->header = header;
			insert_ordered_array((void *)header, &heap->index );
		} else {
			header = lookup_ordered_array( idx, &heap->index );
			header->size += new_length - old_length;

			footer = (footer_t *)((uint32_t)header + header->size - sizeof(footer_t));
			footer->header = header;
			footer->magic = HEAP_MAGIC;
		}

		return alloc( size, align, heap );
	}

	//kputs( "testpoint 8\n" );
	header_t *orig_hole_header = (header_t *)lookup_ordered_array( i, &heap->index ),
		 *hole_header,
		 *block_header;
	footer_t *hole_footer,
		 *block_footer;
	//printf( "testpoint 9, 0x%x\n", &heap->index );


	orig_hole_pos = (uint32_t)orig_hole_header;
	orig_hole_size = orig_hole_header->size;
	if ( orig_hole_size - new_size < sizeof( header_t ) + sizeof( footer_t ) ){
		size += orig_hole_size - new_size;
		new_size = orig_hole_size;
	}

	if ( align && orig_hole_pos & 0xfffff000 ){
		new_location = orig_hole_pos + 0x1000 - ( orig_hole_pos & 0xfff ) - sizeof( header_t );
		hole_header = ( header_t *)orig_hole_pos;
		hole_header->size = 0x1000 - ( orig_hole_pos & 0xfff ) - sizeof( header_t );
		hole_header->magic = HEAP_MAGIC;
		hole_header->is_hole = 1;
		
		hole_footer = (footer_t *)((uint32_t)new_location - sizeof( footer_t ));
		hole_footer->magic = HEAP_MAGIC;
		hole_footer->header = hole_header;
		orig_hole_pos = new_location;
		orig_hole_size = orig_hole_size - hole_header->size;
	} else {
		remove_ordered_array( i, &heap->index );
	}
	block_header 	      = (header_t *)orig_hole_pos;
	block_header->magic   = HEAP_MAGIC;
	block_header->is_hole = 0;
	block_header->size    = new_size;

	block_footer          = (footer_t *)( orig_hole_pos + sizeof( header_t ) + size );
	block_footer->magic   = HEAP_MAGIC;
	block_footer->header  = block_header;

	if ( orig_hole_size - new_size > 0 ){
		hole_header 	     = (header_t *)( orig_hole_pos + sizeof( header_t ) + size + sizeof( footer_t ));
		hole_header->magic   = HEAP_MAGIC;
		hole_header->is_hole = 1;
		hole_header->size    = orig_hole_size - new_size;

		hole_footer	     = (footer_t *)((uint32_t)hole_header + orig_hole_size - new_size - sizeof( footer_t ));
		if ((uint32_t)hole_footer < heap->end_addr ){
			hole_footer->magic = HEAP_MAGIC;
			hole_footer->header = hole_header;
		}
		insert_ordered_array((void *)hole_header, &heap->index );
	}
	
	return (void *)((uint32_t)block_header + sizeof( header_t ));
}

heap_t *create_heap( uint32_t start, uint32_t end_addr, uint32_t max, uint8_t supervisor, uint8_t readonly ){
	heap_t *heap = (heap_t *)kmalloc( sizeof(heap_t), 0, 0);
	assert(( start % 0x1000 ) == 0, (heap_t *)0 );
	assert(( end_addr % 0x1000 ) == 0, (heap_t*)0 );

	heap->index = place_ordered_array((void *)start, HEAP_INDEX_SIZE, &header_t_less_than );
	start += sizeof( type_t ) * HEAP_INDEX_SIZE;

	if (( start & 0xfffff000 ) != 0 ){
		start &= 0xfffff000;
		start += 0x1000;
	}

	heap->start_addr = start;
	heap->end_addr   = end_addr;
	heap->max_addr   = max;
	heap->supervisor = supervisor;
	heap->readonly	 = readonly;

	header_t *hole = (header_t *)start;
	hole->size = end_addr-start;
	hole->magic = HEAP_MAGIC;
	hole->is_hole = 1;
	insert_ordered_array((void *)hole, &heap->index );

	return heap;
}

void free( void *p, heap_t *heap ){
	if ( p == 0 )
		return;

	header_t *header = (header_t *)((uint32_t)p - sizeof( header_t ));
	footer_t *footer = (footer_t *)((uint32_t)header + header->size - sizeof( footer_t ));
	header_t *test_header;
	footer_t *test_footer;
	uint32_t cache_size, i;
	char do_add = 1;

	assert( header->magic == HEAP_MAGIC, );
	assert( footer->magic == HEAP_MAGIC, );

	header->is_hole = 1;
	do_add = 1;

	test_footer = (footer_t *)((uint32_t)header - sizeof( footer_t ));
	if ( test_footer->magic == HEAP_MAGIC && test_footer->header->is_hole == 1 ){
		cache_size = header->size;
		header = test_footer->header;
		header->size += cache_size;
		do_add = 0;
	}
	test_header = (header_t *)((uint32_t)footer + sizeof( footer_t ));
	if ( test_header->magic == HEAP_MAGIC && test_header->is_hole ){
		header->size += test_header->size;
		test_footer = (footer_t *)((uint32_t)test_header + test_header->size - sizeof( footer_t ));
		footer = test_footer;

		for ( i = 0; ( i < heap->index.size ) 
			&& (lookup_ordered_array( i, &heap->index ) != (void *)test_header); i++);

		assert( i < heap->index.size, );

		remove_ordered_array( i, &heap->index );
	}

	if ((uint32_t)footer + sizeof( footer_t ) == heap->end_addr ){
		uint32_t old_length = heap->end_addr - heap->start_addr;
		uint32_t new_length = contract((uint32_t)header - heap->start_addr, heap );

		if ( header->size - ( old_length - new_length ) > 0 ){
			header->size -= old_length - new_length;
			footer = (footer_t *)((uint32_t)header + header->size - sizeof( footer_t ));
			footer->magic = HEAP_MAGIC;
			footer->header = header;
		} else {
			for ( i = 0; ( i < heap->index.size ) && 
				(lookup_ordered_array( i, &heap->index ) != (void *)test_header ); i++ );


			if ( i < heap->index.size ){
				remove_ordered_array( i, &heap->index );
			}
		}
	}
	if ( do_add ){
		insert_ordered_array((void *)header, &heap->index );
	}
}

#endif
