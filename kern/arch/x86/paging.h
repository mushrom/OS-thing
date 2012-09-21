#ifndef _kernel_paging_h
#define _kernel_paging_h

#include <lib/stdint.h>

typedef struct page {
	uint32_t present	: 1;
	uint32_t rw		: 1;
	uint32_t user		: 1;
	uint32_t accessed	: 1;
	uint32_t dirty		: 1;
	uint32_t unused		: 7;
	uint32_t frame		: 20;
} page_t;

typedef struct page_table {
	page_t pages[1024];
} page_table_t;

typedef struct page_directory {
	page_table_t *tables[1024];
	uint32_t tables_phys[1024];
	uint32_t phys_addr;
} page_directory_t;

#include <lib/string.h>
#include <arch/x86/isr.h>
#include <lib/kmacros.h>
#include <arch/x86/kheap.h>
#include <arch/x86/timer.h>

void init_paging();
void switch_page_directory( page_directory_t * );
page_t *get_page( uint32_t, uint8_t, page_directory_t * );
void page_fault( registers_t regs );
void alloc_frame( page_t *, uint8_t, uint8_t );
void free_frame( page_t * );
page_directory_t *clone_directory( page_directory_t * );
extern void copy_page_physical( uint32_t, uint32_t );

#endif
