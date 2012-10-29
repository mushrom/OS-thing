#ifndef _kernel_paging_c
#define _kernel_paging_c

#include <paging.h>
extern unsigned long placement;
unsigned long *page_stack, page_ptr = 0, npages = 0, mem_end = 0x1000000; /*16MB limit hardcoded for now*/
page_dir_t *kernel_dir = 0, *current_dir = 0;
page_table_t *clone_table( page_table_t *src, unsigned long *phys );
page_dir_t *clone_page_dir( page_dir_t *src );
extern void copy_page_phys( unsigned long, unsigned long );
extern struct kmemnode *kheap;

void set_page_dir( page_dir_t *dir ){
	unsigned long cr0;
	current_dir = dir;
	asm volatile( "mov %0, %%cr3":: "r"(dir->address));
	asm volatile( "mov %%cr0, %0": "=r"(cr0));
	asm volatile( "mov %0, %%cr0":: "r"(cr0 | 0x80000000));
}
	
void page_fault_handler( registers_t *regs ){
	unsigned long fault_addr;
	asm volatile( "mov %%cr2, %0": "=r"(fault_addr));

	printf( "page fault: address 0x%x%s%s%s%s\n",
		fault_addr,
		(!regs->err_code & 0x1 )?" not present":"",
		(regs->err_code & 0x2 )?" read-only":"",
		(regs->err_code & 0x4 )?" user-mode":"",
		(regs->err_code & 0x8 )?" reserved":""
	);
	dump_registers( regs );
	printf( "pid: %d\n", getpid());
	end_bad_task( );
}

void alloc_page( unsigned long *address, unsigned int permissions ){
	*address = pop_page() | permissions;
}

void set_page( unsigned long *address, unsigned long virtual, unsigned int permissions ){
	*address = virtual | permissions;
}

void set_table_perms( unsigned int permissions, unsigned long address, page_dir_t *dir ){
	unsigned long 	high_index = address / 0x1000,
			low_index  = high_index / 1024;
	dir->tables[ low_index ] = (void *)((unsigned long)dir->tables[low_index] | permissions );
}

unsigned long *get_page( unsigned long address, unsigned char make, page_dir_t *dir ){
	unsigned long 	high_index = address / 0x1000,
			low_index  = high_index / 1024;

	if ( !dir->tables[ low_index ] && make ){
		unsigned long tmp;
		dir->tables[ low_index ] = (void *)kmalloc( sizeof( page_table_t ), 1, &tmp );
		memset( dir->tables[low_index], 0, PAGE_SIZE );
		dir->table_addr[ low_index ] = tmp | PAGE_USER | PAGE_WRITEABLE | PAGE_PRESENT;
	} 
	
	return &dir->tables[ low_index ]->address[ high_index % 1024 ];
}

void push_page( unsigned long address ){
	page_stack[ page_ptr++ ] = address & 0xfffff000;
}

unsigned long pop_page( void ){
	if ( page_ptr ){
		return page_stack[ --page_ptr ];
	} else {
		PANIC( "out of free pages" );
	}
	return 0;
}

void map_pages( unsigned long start, unsigned long end, unsigned int permissions, page_dir_t *dir ){
	unsigned long address = 0, *i;
	for ( address = start; address < end; address += 0x1000 ){
		i = get_page( address, 1, dir ), permissions; 
		if ( *i ){
			set_page( i, address, permissions );
		} else {
			alloc_page( i, permissions );
		}
		//printf( "0x%x:0x%x\n", i, *i );
	}

	unsigned long 	high_index = address / 0x1000,
			low_index  = high_index / 1024;
	for ( ; low_index * 0x1000 < end; low_index++ )
		set_table_perms( permissions, low_index * 0x1000, dir );
}

void free_pages( unsigned long start, unsigned long end, page_dir_t *dir ){
	unsigned long address, *i;
	for ( address = start; address < end; address += 0x1000 ){
		i = get_page( address, 0, dir );
		printf( "Free'd page... 0x%x\n", *i );
		push_page( *i );
	}
}

void init_paging( ){
	unsigned long address	= 0, i;
	page_dir_t   *kernel_dir	  = (void *)kmalloc_e( sizeof( page_dir_t ), 1, &address );
		      kernel_dir->address = (void *)address;
	npages			= mem_end / PAGE_SIZE;
	page_stack 	 	= (void *)kmalloc_e( npages, 1, 0 );

	for ( i = mem_end; i + PAGE_SIZE > 0; i -= PAGE_SIZE )
		push_page( i );

	for ( i = 1; i < 1024; i++ )
		kernel_dir->tables[i] = 0;

	map_pages( 0,		placement + PAGE_SIZE * 5, PAGE_USER | PAGE_WRITEABLE | PAGE_PRESENT, kernel_dir );
	map_pages( KHEAP_START, KHEAP_START + KHEAP_SIZE,  PAGE_USER | PAGE_WRITEABLE | PAGE_PRESENT, kernel_dir );

	register_interrupt_handler( 0xe, page_fault_handler );

	set_page_dir( kernel_dir );

	kheap = (void *)init_heap( KHEAP_START, KHEAP_SIZE, kernel_dir );
	printf( "    %d total pages, %d allocated for kernel (%d free, last at 0x%x)\n", 
			npages, npages - page_ptr, page_ptr, address );

}

/*
page_dir_t *clone_page_dir( page_dir_t *src ){
	unsigned long phys;
	unsigned long offset;
	unsigned long i;

	page_dir_t *dir = (page_dir_t *)kmalloc( sizeof( page_dir_t ), 1, &phys );
	memset( dir, 0, sizeof( page_dir_t ));

	offset = (unsigned long)dir->table_addr - (unsigned long)dir;
	dir->address = (void *)phys + offset;

	printf( "[1] Got here\n" );
	for ( i = 0; i < 1024; i++ ){
		if ( src->tables[i] ){
			if ( kernel_dir->tables[i] == src->tables[i] ){
				printf( "[1.1] Got here\n" );
				dir->tables[i] = src->tables[i];
				dir->table_addr[i] = src->table_addr[i];
			} else {
				dir->tables[i] = clone_table( src->tables[i], &phys );
				dir->table_addr[i] = phys | PAGE_USER | PAGE_WRITEABLE | PAGE_PRESENT;
			}
			//set_table_perms( PAGE_USER | PAGE_WRITEABLE | PAGE_PRESENT, (unsigned long)dir->tables[i], current_dir );
		}
	}
	printf( "[4] Got here\n" );
	return dir;
}

page_table_t *clone_table( page_table_t *src, unsigned long *phys ){
	int i;
	page_table_t *table = (page_table_t *)kmalloc( sizeof( page_table_t ), 1, phys );
	memset( table, 0, sizeof( page_dir_t ));

	printf( "[2] Got here\n" );
	for ( i = 0; i < 1024; i++ ){
		if ( src->address[i] ){
			alloc_page( &table->address[i] );
			printf( "[2.1] Got here\n" );
			table->address[i] |= src->address[i] & 31;
			copy_page_phys( src->address[i], table->address[i] );
			printf( "[2.2] Got here\n" );
		}
	}
	printf( "[3] Got here\n" );
	return table;
}
*/


#endif
