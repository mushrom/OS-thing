#ifndef _kernel_paging_h
#define _kernel_paging_h
#include <arch/x86/isr.h>
#include <lib/stdint.h>
#include <lib/kmacros.h>
#include <mem/alloc.h>

#define PAGE_USER 	4 	/* USER		*/
#define PAGE_WRITEABLE 	2 	/* read/write	*/
#define PAGE_PRESENT 	1	/* present	*/
#define PAGE_SIZE	0x1000	/* 4kb		*/

typedef struct page_table {
	unsigned long address[1024];
} page_table_t;

typedef struct page_dir {
	page_table_t *tables[1024];
	unsigned long table_addr[1024];
	struct page_dir *address;
} page_dir_t;


void set_table_perms( unsigned int, unsigned long, page_dir_t * );
void init_heap( unsigned long start, unsigned long size, page_dir_t * );
unsigned long *get_page( unsigned long, unsigned char, page_dir_t * );
unsigned long pop_page( void );
void alloc_page( unsigned long * );
void push_page( unsigned long );
void init_paging();

#endif
