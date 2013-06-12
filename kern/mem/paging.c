#ifndef _kernel_paging_c
#define _kernel_paging_c

#include <paging.h>
extern unsigned long 	placement,
			initial_esp;
unsigned long 	*page_stack, 
		page_ptr = 0, 
		stack_bottom = 0, 
		npages = 0, 
		mem_end = 0x1000000; 

page_dir_t 	*kernel_dir = 0, 
		*current_dir = 0;

task_t 		*current_task;

//page_table_t *clone_table( page_table_t *src, unsigned long *phys );
//page_dir_t *clone_page_dir( page_dir_t *src );
extern void copy_page_phys( unsigned long, unsigned long );
extern struct heap *kheap;

/** \brief Set the page directory 
 * @param dir Pointer to the page directory to switch to
 */
void set_page_dir( page_dir_t *dir ){
	unsigned long cr0;
	current_dir = dir;
	asm volatile( "mov %0, %%cr3":: "r"(dir->address));
	asm volatile( "mov %%cr0, %0": "=r"(cr0));
	asm volatile( "mov %0, %%cr0":: "r"(cr0 | 0x80000000));
}

/** \brief Flush all tlb entries */
void flush_tlb( void ){
	asm volatile( "mov %cr3, %eax" );
	asm volatile( "mov %eax, %cr3" );
}
	
/** \brief Page fault isr handler */
void page_fault_handler( registers_t *regs ){
	unsigned long fault_addr;
	asm volatile( "mov %%cr2, %0": "=r"(fault_addr));
	memmap_t *map;

	if (( map = memmaps_check( current_task->maps, fault_addr )) && get_page( fault_addr, current_dir ) == 0 ){
		printf( "Address 0x%x is in task's address space, mapping...\n", fault_addr );
		map_page( fault_addr, PAGE_USER | PAGE_WRITEABLE | PAGE_PRESENT, current_dir );
		flush_tlb();
	} else {
		printf( "page fault: address 0x%x%s%s%s%s\n",
			fault_addr,
			(!regs->err_code & 0x1 )?" not present":"",
			(regs->err_code & 0x2 )?" read-only":"",
			(regs->err_code & 0x4 )?" user-mode":"",
			(regs->err_code & 0x8 )?" reserved":""
		);
		dump_registers( regs );
		printf( "pid: %d\n", getpid());

		signal_bad_task( SIGSEGV );

		PANIC( "Page fault" );
	}
}

/** \brief Push a page onto the page stack
 * @param address The address to push on to stack
 */
void push_page( unsigned long address ){
		//printf( "pushed page 0x%x, %d:%d\n", address, page_ptr, npages );
		page_stack[ page_ptr++ ] = address & 0xfffff000;
}

/** \brief Pop a page off the page stack
 * @return The page address if successful, will panic the kernel otherwise
 */
unsigned long pop_page( void ){
	if ( page_ptr > stack_bottom ){
		return page_stack[ --page_ptr ];
	} else {
		PANIC( "out of free pages" );
		while( 1 ) asm ("hlt");;
	}
	PANIC( "Why am I here? :(" );
	return 0;
}

unsigned long pop_bottom_page( void ){
	if ( stack_bottom < npages )
		return page_stack[ stack_bottom++ ];
	else 
		PANIC( "out of free pages" );

	return 0;
}

/** \brief Perform basic sanity checks on the page stack.
 * This function is intended for debugging.
 * If enabled, it will add a relatively significant amount of time to allocating pages,
 * as it checks each time it maps a new page. 
 */
void check_pstack( void ){
	int i;
	for ( i = 0; i < npages - 200; i++ ){
		if ( !page_stack[i] ){
			printf( "[\x14-\x17] Failed page stack check!\n" );
			printf( "Bad page %d, 0x%x!\n", i, &page_stack[i] );
			printf( "before: 0x%x 0x%x 0x%x 0x%x\n"
				"after : 0x%x 0x%x 0x%x 0x%x\n",
					page_stack[i-1], page_stack[i-2], page_stack[i-3], page_stack[i-4],
					page_stack[i+1], page_stack[i+2], page_stack[i+3], page_stack[i+4]
			);
					
			page_stack[i] = 1;
			if ( page_stack[i] )
				printf( "could write to stack\n" );
			PANIC( "Panicing, debug this." );
			while( 1 ) asm( "hlt" );
		}
	}
}

/** \brief Maps a virtual address to a page, allocates a new frame if not already allocated
 * @param address The virtual address to assign
 * @param permissions The permissions for the new page
 * @param dir The page directory to add the entry to
 */
void map_page( unsigned long address, unsigned int permissions, page_dir_t *dir ){
	unsigned long 	low_index = address >> 22,
			high_index  = address >> 12 & 0x3ff,
			temp = 0;

	if ( !dir->tables[ low_index ] ){
		dir->tables[ low_index ] = (void *)kmalloc( sizeof( page_table_t ), 1, &temp );
		memset( dir->tables[ low_index ], 0, sizeof( page_table_t ));
		dir->table_addr[ low_index ] = temp | permissions;
	}

	dir->tables[ low_index ] = (void *)((unsigned long)dir->tables[ low_index ] & 0xfffff000);

	if ( !dir->tables[ low_index ]->address[ high_index ] )
		dir->tables[ low_index ]->address[ high_index ] = pop_page();

	dir->tables[ low_index ]->address[ high_index ] |= permissions;
	dir->tables[ low_index ] = (void *)((unsigned long)dir->tables[ low_index ] | permissions );
}

/** \brief Maps a virtual address to a given real address
 * @param address The virtual address to assign
 * @param real The real address to map to
 * @param permissions The permissions for the new page
 * @param dir The page directory to add the entry to
 */
void map_r_page( unsigned long address, unsigned long real, unsigned int permissions, page_dir_t *dir ){
	unsigned long 	low_index = address >> 22,
			high_index  = address >> 12 & 0x3ff,
			temp = 0;

	if ( !dir->tables[ low_index ] ){
		dir->tables[ low_index ] = (void *)kmalloc( sizeof( page_table_t ), 1, &temp );
		memset( dir->tables[ low_index ], 0, sizeof( page_table_t ));
		dir->table_addr[ low_index ] = temp | permissions;
	}

	dir->tables[ low_index ] = (void *)((unsigned long)dir->tables[ low_index ] & 0xfffff000);

	dir->tables[ low_index ]->address[ high_index ] = real;

	dir->tables[ low_index ]->address[ high_index ] |= permissions;
	dir->tables[ low_index ] = (void *)((unsigned long)dir->tables[ low_index ] | permissions );
}

/** \brief "Rawly" maps a virtual address to a given real address, doesn't alter \ref real or table permissions.
 * @param address The virtual address to assign
 * @param real The real address to map to
 * @param dir The page directory to add the entry to
 */
void rmap_r_page( unsigned long address, unsigned long real, page_dir_t *dir ){
	unsigned long 	low_index = address >> 22,
			high_index  = address >> 12 & 0x3ff,
			temp = 0;

	if ( !dir->tables[ low_index ] ){
		dir->tables[ low_index ] = (void *)kmalloc( sizeof( page_table_t ), 1, &temp );
		memset( dir->tables[ low_index ], 0, sizeof( page_table_t ));
		dir->table_addr[ low_index ] = temp | ( real & 7 );
	}

	dir->tables[ low_index ] = (void *)((unsigned long)dir->tables[ low_index ] & 0xfffff000);

	dir->tables[ low_index ]->address[ high_index ] = real;
	dir->tables[ low_index ] = (void *)((unsigned long)dir->tables[ low_index ] | ( real & 7 ));
}

/** \brief Look up a physical page address
 * @param address The virtual address to look up
 * @param dir The page directory to look in
 * @return The physical address if found, 0 otherwise
 */
unsigned long get_page( unsigned long address, page_dir_t *dir ){
	unsigned long 	low_index = address >> 22,
			high_index  = address >> 12 & 0x3ff,
			ret_addr;
	page_table_t *temp;
	//printf( "0x%x\n", tdmp );

	temp = (void *)((unsigned long)dir->tables[ low_index ] & 0xfffff000 );
	if ( !temp )
		return 0;

	ret_addr = (unsigned long)temp->address[ high_index % 1024 ];

	return ret_addr;
}

/** \brief Unmap a page and push it back on the stack
 * @param address The virtual address to free
 * @param dir The page directory to free from
 */
void free_page( unsigned long address, page_dir_t *dir ){
	unsigned long 	low_index = address >> 22,
			high_index  = address >> 12 & 0x3ff;

	if ( !dir->tables[ low_index ] )
		return;

	dir->tables[ low_index ]->address[ high_index % 1024 ] = 0;
	push_page( address & 0xfffff000 );
}

/** \brief Map a range of addresses
 * @param start The starting virtual address
 * @param end The ending virtual address
 * @param permissions Permissions to assign to the pages
 * @param dir The page directory to work in
 */
void map_pages( unsigned long start, unsigned long end, unsigned int permissions, page_dir_t *dir ){
	unsigned long address = 0;
	for ( address = start; address < end; address += 0x1000 )
		map_page( address, permissions, dir );
}

/** \brief Map a range of addresses
 * @param start The starting virtual address
 * @param end The ending virtual address
 * @param real The starting real address
 * @param permissions Permissions to assign to the pages
 * @param dir The page directory to work in
 */
void map_r_pages( unsigned long start, unsigned long end, unsigned long real, unsigned int permissions, page_dir_t *dir ){
	unsigned long address = start, r_address = real;
	for ( ; address < end; address += 0x1000, r_address += 0x1000 )
		map_r_page( address, r_address, permissions, dir );
}

/** \brief Free a range of addresses
 * @param start The starting virtual address
 * @param end The ending virtual address
 * @param dir The page directory to work in
 */
void free_pages( unsigned long start, unsigned long end, page_dir_t *dir ){
	unsigned long address = 0;
	for ( address = start; address < end; address += 0x1000 )
		free_page( address, dir );
}

/** \brief Set up all the needed pages.
 * Maps the kernel to itself, and the kernel heap.
 */
void init_paging( multiboot_header_t *mboot_h ){
	unsigned long address	= 0, i;
	unsigned long max_mem = ( mboot_h->mem_lower + mboot_h->mem_upper ) & 0xfffff000; // maximum memory in kb

	kernel_dir		= (void *)kmalloc_e( sizeof( page_dir_t ), 1, &address );
	kernel_dir->address 	= (void *)address;

	npages			= max_mem / 4;
	page_stack 	 	= (void *)kmalloc_e( npages * sizeof( int ) + 1, 1, 0 );

	for ( i = max_mem * 1024; i + PAGE_SIZE > 0; i -= PAGE_SIZE )
		push_page( i );

	for ( i = 1; i < 1024; i++ )
		kernel_dir->tables[i] = 0;

	map_pages( 0, placement + PAGE_SIZE * 400,  PAGE_USER | PAGE_WRITEABLE | PAGE_PRESENT, kernel_dir );
	map_pages( KHEAP_START, KHEAP_START + KHEAP_SIZE + PAGE_SIZE,   PAGE_USER | PAGE_WRITEABLE | PAGE_PRESENT, kernel_dir );

	register_interrupt_handler( 0xe, page_fault_handler );
	kernel_dir->address = (void *)((unsigned long)kernel_dir->address | 7);

	set_page_dir( kernel_dir );

	/* small test for page linking 
	map_page( 0xd0000000, 7, kernel_dir );
	rmap_r_page( 0xe0000000, get_page( 0xd0000000, kernel_dir ), kernel_dir );
	set_page_dir( kernel_dir );
	flush_tlb();

	memcpy( 0xd0000000, "hello\n", 7 );
	printf( "%s", (char *)0xe0000000 );
	*/
	kheap = (void *)init_heap( KHEAP_START, KHEAP_SIZE );
	printf( "%d total pages, %d allocated for kernel (%d free, last at 0x%x)\n", 
			npages, npages - page_ptr, page_ptr, address );
	/*
	printf( "    placement: 0x%x\n", placement );
	printf( "    kheap size: 0x%x\n", KHEAP_SIZE );
	printf( "    meh: 0x%x\n", sizeof( page_table_t ) * npages );
	*/
	//set_page_dir( kernel_dir );
	flush_tlb();


	//*(unsigned long *)(KHEAP_START + KHEAP_SIZE) = 1;
}

page_table_t *clone_table( page_table_t *src, unsigned long *phys_addr ){
	page_table_t *table = (page_table_t *)kmalloc( sizeof( page_table_t ), 1, phys_addr );
	unsigned long i, src_flags;

	for ( i = 0; i < 1024; i++ ){
		if ( src->address[i] ){
			src_flags = src->address[i] & 0xfff;
			table->address[i] = pop_page();

			copy_page_phys( src->address[i] & 0xfffff000, table->address[i] & 0xfffff000 );
			table->address[i] = table->address[i] | PAGE_USER | PAGE_WRITEABLE | PAGE_PRESENT;
			printf( "cloned page 0x%x->0x%x\n", src->address[i] &0xfffff000, table->address[i] & 0xfffff000 );
		}
	}
	return table;
}

page_dir_t *clone_page_dir( page_dir_t *src ){
	unsigned long phys, offset, i;

	page_dir_t *dir = (page_dir_t *)kmalloc_e( sizeof( page_dir_t ), 1, &phys );
	memset( dir, 0, sizeof( page_dir_t ));

	offset = (unsigned long)dir->table_addr - (unsigned long)dir;
	dir->address = (void *)phys + offset;

	for ( i = 0; i < 1024; i++ ){
		if ( src->tables[i] ){
				dir->tables[i] 	   = src->tables[i];
				dir->table_addr[i] = src->table_addr[i];
		}
	}

	return dir;
}

/* 
void move_stack( unsigned long start ){
	unsigned long i, j, size = 0x1000;
	unsigned long old_esp, old_ebp, offset, new_esp, new_ebp;

	asm volatile( "mov %%esp, %0" : "=r" (old_esp));
	asm volatile( "mov %%ebp, %0" : "=r" (old_ebp));

	i = start;
	j = old_esp;
	printf( "j: 0x%x i: 0x%x size: 0x%x\n", j, i, size );
	for ( ; i >= start - size; j -= 0x1000, i -= 0x1000 ){
	}
	printf( "meh\n" );
	set_page_dir( kernel_dir );
	flush_tlb();
	
	asm volatile( "mov %%esp, %0" : "=r" (old_esp));
	asm volatile( "mov %%ebp, %0" : "=r" (old_ebp));

	offset = start - initial_esp;
	new_esp = old_esp + offset;
	new_ebp = old_ebp + offset;

	map_page( 0xe0000000, 7, kernel_dir );
	//printf( "dir: 0x%x\n", kernel_dir );

	///set_page_dir( kernel_dir );
	flush_tlb();


	//printf( "new esp: 0x%x offset: 0x%x\n", new_esp, offset );
	
	//asm volatile( "mov %0, %%esp" :: "r"( new_esp ));
	//asm volatile( "mov %0, %%ebp" :: "r"( new_ebp ));
	printf( "0x%x\n", *(unsigned *)0xe0000000 );

}
*/
#endif
