#ifndef _kernel_paging_c
#define _kernel_paging_c
#include <arch/x86/paging.h>

uint32_t *frames;
uint32_t nframes;
extern uint32_t placement_address;
extern heap_t *kheap;
page_directory_t *kernel_directory = 0, *current_directory = 0;

#define INDEX_FROM_BIT(a) (a/(8*4))
#define OFFSET_FROM_BIT(a) (a%(8*4))

static void set_frame( uint32_t frame_addr ){
	uint32_t frame = frame_addr/0x1000;
	uint32_t idx   = INDEX_FROM_BIT( frame );
	uint32_t off   = OFFSET_FROM_BIT( frame );
	frames[idx]   |= ( 0x1 << off );
}

static void clear_frame( uint32_t frame_addr ){
	uint32_t frame = frame_addr/0x1000;
	uint32_t idx   = INDEX_FROM_BIT( frame );
	uint32_t off   = OFFSET_FROM_BIT( frame );
	frames[idx]   &= ~(0x1 << off );
}

static page_table_t *clone_table( page_table_t *src, uint32_t *phys ){
	//kputs( "testpoint 4\n" );
	page_table_t *table = (page_table_t *)kmalloc_ap( sizeof( page_table_t ), phys );
	memset( table, 0, sizeof( page_directory_t ));
	//kputs( "testpoint 5\n" );

	//printf( "phys3: 0x%x, 0x%x\n", &phys, phys );

	int i;
	for ( i = 0; i < 1024; i++ ){
		if ( src->pages[i].frame ){
		
			alloc_frame( &table->pages[i], 0, 0 );

			if ( src->pages[i].present )	table->pages[i].present = 1;
			if ( src->pages[i].rw )		table->pages[i].rw = 1;
			if ( src->pages[i].user )	table->pages[i].user = 1;
			if ( src->pages[i].accessed )	table->pages[i].accessed = 1;
			if ( src->pages[i].dirty ) 	table->pages[i].dirty = 1;

			copy_page_physical( src->pages[i].frame * 0x1000, table->pages[i].frame * 0x1000 );
		}
	}
	return table;
}

/* Commented out for the moment, until it's needed 
static uint32_t test_frame( uint32_t frame_addr ){
	uint32_t frame = frame_addr/0x1000;
	uint32_t idx   = INDEX_FROM_BIT( frame );
	uint32_t off   = OFFSET_FROM_BIT( frame );

	return ( frames[idx] & ( 0x1 << off ));
}
*/

static uint32_t first_frame(){
	uint32_t i = 0, j = 0;
	for ( i = 0; i < INDEX_FROM_BIT( nframes ); i++ ){
		if ( frames[i] != 0xffffffff ){
			for ( j = 0; j < 32; j++ ){
				uint32_t toTest = 0x1 << j;
				if ( !(frames[i]&toTest )){
					return i*4*8+j;
				}
			}
		}
	}
	return (uint32_t)-1;
}

void alloc_frame( page_t *page, uint8_t is_kernel, uint8_t is_writable ){
	if ( page->frame != 0 ){
		return;
	} else {
		uint32_t idx = first_frame();
		if ( idx == (uint32_t)-1 ){
			PANIC( "No free frames...\n" );
		}
		set_frame( idx*0x1000 );
		page->present = 1;
		page->rw = (is_writable == 1)?1:0;
		page->user = (is_kernel == 1)?0:1;
		page->frame = idx;
	}
}

void free_frame( page_t *page ){
	uint32_t frame;
	if ( !(frame = page->frame )){
		return;
	} else {
		clear_frame( frame );
		page->frame = 0;
	}
}

void init_paging(){
	uint32_t mem_end_page = 0x1000000, 
		 i = 0;
		 //phys;

	nframes = mem_end_page / 0x1000;
	frames  = kmalloc( INDEX_FROM_BIT( nframes ), 0, 0 );
	memset( frames, 0, INDEX_FROM_BIT( nframes ));

	kernel_directory = ( page_directory_t *)kmalloc_a( sizeof( page_directory_t ));
	memset( kernel_directory, 0, sizeof( page_directory_t ));
	//current_directory = kernel_directory;
	kernel_directory->phys_addr = (uint32_t)kernel_directory->tables_phys;


	for ( i = KHEAP_START; i < KHEAP_START + KHEAP_INIT_SIZE; i += 0x1000 )
		get_page( i, 1, kernel_directory );

	i = 0;
	while ( i < placement_address + 0x1000 ){
		alloc_frame( get_page( i, 1, kernel_directory ), 0, 0 );
		i += 0x1000;
	}

	for ( i = KHEAP_START; i < KHEAP_START + KHEAP_INIT_SIZE; i += 0x1000 )
		alloc_frame( get_page( i, 1, kernel_directory ), 0, 0 );

	register_interrupt_handler( 14, page_fault );
	register_interrupt_handler( 13, page_fault );
	switch_page_directory( kernel_directory );

	kheap = create_heap( KHEAP_START, KHEAP_START + KHEAP_INIT_SIZE, 0xcffff000, 0, 0 );

	current_directory = clone_directory( kernel_directory );
	switch_page_directory( current_directory );
	//switch_page_directory( kernel_directory );
}

void switch_page_directory( page_directory_t *dir ){
	current_directory = dir;
	asm volatile ( "mov %0, %%cr3":: "r"( dir->phys_addr ));
	uint32_t cr0;
	asm volatile ( "mov %%cr0, %0": "=r"(cr0));
	cr0 |= 0x80000000;
	asm volatile ( "mov %0, %%cr0":: "r"(cr0));
}

page_t *get_page( uint32_t address, uint8_t make, page_directory_t *dir ){
	address /= 0x1000;
	uint32_t table_idx = address / 1024;
	if ( dir->tables[ table_idx ]){
		return &dir->tables[ table_idx ]->pages[address % 1024];
	} else if ( make ){
		uint32_t tmp;
		dir->tables[table_idx] = (page_table_t *)kmalloc_ap( sizeof( page_table_t ), &tmp );
		memset( dir->tables[table_idx], 0, 0x1000 );
		dir->tables_phys[ table_idx ] = tmp | 0x7;
		return &dir->tables[ table_idx ]->pages[ address % 1024 ];
	} else {
		return 0;
	}
}

page_directory_t *clone_directory( page_directory_t *src ){
	uint32_t offset;
	uint32_t phys;
	int i;
	
	page_directory_t *dir = (page_directory_t *)kmalloc_ap( sizeof( page_directory_t ), &phys );
	memset( dir, 0, sizeof( page_directory_t ));

	offset = (uint32_t)dir->tables_phys - (uint32_t)dir;
	dir->phys_addr = phys + offset;
	//printf( "phys1 = 0x%x, 0x%x\n"
		//"dir = 0x%x, dir->phys_addr = 0x%x\n",
		//&phys, phys, dir, dir->phys_addr );

	for ( i = 0; i < 1024; i++ ){
		if ( !src->tables[i] )
			continue;

		if ( kernel_directory->tables[i] == src->tables[i] ){
			dir->tables[i] = src->tables[i];
			dir->tables_phys[i] = src->tables_phys[i];
		} else {
			uint32_t phys;
			//kputs( "testpoint 3\n" );
			dir->tables[i] = clone_table( src->tables[i], &phys );
			//printf( "phys2 = 0x%x, 0x%x\n", &phys, phys );
			dir->tables_phys[i] = phys | 0x07;
		}
	}
	return dir;
}

void page_fault( registers_t regs ){
	uint32_t fault_addr;
	asm volatile( "mov %%cr2, %0" : "=r" ( fault_addr ));

	unsigned int 	present	= !(regs.err_code & 0x1 ),
			rw	= regs.err_code & 0x2,
			us	= regs.err_code & 0x4,
			reserved= regs.err_code & 0x8,
			id 	= regs.err_code & 0x10;	

	printf( "Page fault: %s%s%s%sat 0x%x, %x\n",
		(present)? 	"present ":"",
		(rw)?		"read-only ":"",
		(us)? 		"user-mode ":"",
		(reserved)?	"reserved " :"",
 		fault_addr, id 
	);
	dump_registers( regs );
	PANIC( "Page fault" );
}
#endif
