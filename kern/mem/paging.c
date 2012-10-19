#ifndef _kernel_paging_c
#define _kernel_paging_c

#include <mem/paging.h>
extern unsigned long placement;
unsigned long *page_stack, page_ptr = 0, npages = 0, mem_end = 0x1000000; /*16MB limit hardcoded for now*/
page_dir_t *kernel_dir = 0, *current_dir = 0;

void set_page_dir( page_dir_t *dir ){
	unsigned long cr0;
	asm volatile( "mov %0, %%cr3":: "r"(dir->address));
	asm volatile( "mov %%cr0, %0": "=r"(cr0));
	asm volatile( "mov %0, %%cr0":: "r"(cr0 | 0x80000000));
}
	
void page_fault_handler( registers_t regs ){
	unsigned long fault_addr;
	asm volatile( "mov %%cr2, %0": "=r"(fault_addr));

	printf( "page fault: %s%s%s%sat 0x%x\n",
		(!regs.err_code & 0x1 )?"not present ":"",
		(regs.err_code & 0x2 )?"read-only ":"",
		(regs.err_code & 0x4 )?"user-mode ":"",
		(regs.err_code & 0x8 )?"reserved ":"",
		fault_addr );
	dump_registers( regs );
	PANIC( "page fault" );
}

void alloc_page( unsigned long *address ){
	//printf( "Alloc'd 0x%x, %d\n", *address = pop_page() | PAGE_WRITEABLE | PAGE_PRESENT, page_ptr );
	//usleep( 100 );
	*address = pop_page() | PAGE_WRITEABLE | PAGE_PRESENT;
}

void set_table_perms( unsigned int permissions, unsigned long address, page_dir_t *dir ){
	unsigned long 	high_index = address / 0x1000,
			low_index  = high_index / 1024;
	//dir->tables[ low_index ] = (void *)((unsigned long)dir->tables[low_index] & 0xfffff000  );
	dir->tables[ low_index ] = (void *)((unsigned long)dir->tables[low_index] | permissions );
	//printf( "%0x%x\n", ((unsigned long)dir->tables[ low_index ] | permissions ));
}

unsigned long *get_page( unsigned long address, unsigned char make, page_dir_t *dir ){
	unsigned long 	high_index = address / 0x1000,
			low_index  = high_index / 1024;

	if ( !dir->tables[ low_index ] && make ){
		unsigned long tmp;
		dir->tables[ low_index ] = (void *)kmalloc( sizeof( page_table_t ), 1, &tmp );
		memset( dir->tables[low_index], 0, PAGE_SIZE );
		dir->table_addr[ low_index ] = tmp | PAGE_WRITEABLE | PAGE_PRESENT;
	} 
	
	return &dir->tables[ low_index ]->address[ high_index % 1024 ];

	return 0;
}

void push_page( unsigned long address ){
	page_stack[ page_ptr++ ] = address;
}

unsigned long pop_page( void ){
	if ( page_ptr ){
		//if ( page_stack[ page_ptr-1 ] == 0 )
			//printf( "Got null page. 0_o\n" );
		return page_stack[ --page_ptr ];
	} else {
		PANIC( "out of free pages" );
	}
	return 0;
}

void init_paging( ){
	unsigned long address	= 0, i;
	page_dir_t   *kernel_dir= (void *)kmalloc_e( sizeof( page_dir_t ), 1, &address );
		      kernel_dir->address = (void*)address;
	npages			= mem_end / PAGE_SIZE;
	page_stack 		= (void *)kmalloc_e( npages, 1, 0 );

	for ( i = mem_end; i + PAGE_SIZE > 0; i -= PAGE_SIZE ){
		push_page( i );
		//printf( "Pushed page 0x%x\n", i );
	}
	for ( i = 1; i < 1024; i++ ){
		kernel_dir->tables[i] = 0;
	}
	for ( address = KHEAP_START; address < KHEAP_START + KHEAP_SIZE; address += PAGE_SIZE ){ DEBUG_HERE
		get_page( address, 1, kernel_dir );
	}
	for ( address = 0; address + PAGE_SIZE < placement + PAGE_SIZE; address += 0x1000 ){
		alloc_page(get_page( address, 1, kernel_dir ));
	}
	for ( address = KHEAP_START; address < KHEAP_START + KHEAP_SIZE; address += PAGE_SIZE ){ DEBUG_HERE
		alloc_page(get_page( address, 1, kernel_dir ));
	}
	for ( address = 0; address + PAGE_SIZE < placement + PAGE_SIZE; address += 0x1000 ){
		set_table_perms( PAGE_WRITEABLE | PAGE_PRESENT, address, kernel_dir );
	}
	for ( address = KHEAP_START; address < KHEAP_START + KHEAP_SIZE; address += PAGE_SIZE ){ DEBUG_HERE
		set_table_perms( PAGE_USER | PAGE_WRITEABLE | PAGE_PRESENT, address, kernel_dir );
	}
	init_heap( KHEAP_START, KHEAP_SIZE, kernel_dir );	printf( "    initialised heap\n" );

	printf( "    %d total pages, %d allocated for kernel (%d free, last at 0x%x)\n", 
			npages, npages - page_ptr, page_ptr, address );

	register_interrupt_handler( 0xe, page_fault_handler );
	set_page_dir( kernel_dir );
}

#endif
