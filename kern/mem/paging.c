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

void flush_tlb( void ){
	asm volatile( "mov %cr3, %eax" );
	asm volatile( "mov %eax, %cr3" );
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

void set_table_perms( unsigned int permissions, unsigned long address, page_dir_t *dir ){
	unsigned long high_index = address / 0x1000,
			low_index = high_index / 1024;

	dir->tables[ low_index ] = (void *)((unsigned long)dir->tables[ low_index ] | permissions );
}

void map_page( unsigned long address, unsigned int permissions, page_dir_t *dir ){
	unsigned long 	low_index = address >> 22,
			high_index  = address >> 12 & 0x3ff,
			temp = 0;

	if ( !dir->tables[ low_index ] ){
		dir->tables[ low_index ] = (void *)kmalloc( sizeof( page_table_t ), 1, &temp );
		memset( dir->tables[ low_index ], 0, PAGE_SIZE );
		dir->table_addr[ low_index ] = temp | permissions;
	}
	dir->tables[ low_index ] = (void *)((unsigned long)dir->tables[ low_index ] & 0xfffff000);
	dir->tables[ low_index ]->address[ high_index % 1024 ] = pop_page() | permissions;

	dir->tables[ low_index ] = (void *)((unsigned long)dir->tables[ low_index ] | permissions);
	
}

void free_page( unsigned long address, page_dir_t *dir ){
	unsigned long 	low_index = address >> 22,
			high_index  = address >> 12 & 0x3ff,
			temp = 0;

	if ( !dir->tables[ low_index ] )
		return;

	dir->tables[ low_index ]->address[ high_index % 1024 ] = 0;
}

void map_pages( unsigned long start, unsigned long end, unsigned int permissions, page_dir_t *dir ){
	unsigned long address = 0;
	for ( address = start; address < end; address += 0x1000 )
		map_page( address, permissions, dir );
}

void free_pages( unsigned long start, unsigned long end, page_dir_t *dir ){
	unsigned long address = 0;
	for ( address = start; address < end; address += 0x1000 )
		free_page( address, dir );
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

	/* "PAGE_SIZE * 5" is to provide 20kb of heap for kmalloc_e */
	map_pages( 0,		placement + PAGE_SIZE * 5, PAGE_USER | PAGE_WRITEABLE | PAGE_PRESENT, kernel_dir );
	map_pages( KHEAP_START, KHEAP_START + KHEAP_SIZE,  PAGE_USER | PAGE_WRITEABLE | PAGE_PRESENT, kernel_dir );

	register_interrupt_handler( 0xe, page_fault_handler );
	//current_dir = clone_page_dir( kernel_dir );
	kernel_dir->address = (unsigned long)kernel_dir->address | 7;
	set_page_dir( kernel_dir );

	kheap = (void *)init_heap( KHEAP_START, KHEAP_SIZE, kernel_dir );
	printf( "    %d total pages, %d allocated for kernel (%d free, last at 0x%x)\n", 
			npages, npages - page_ptr, page_ptr, address );

	map_pages( 0xa0000000,	0xa000a000, PAGE_USER | PAGE_WRITEABLE | PAGE_PRESENT, kernel_dir );
	flush_tlb();

	//memcpy((void *)0xa0001000, "test", 5 );
}

#endif
