#ifndef _kernel_paging_h
#define _kernel_paging_h
#include <kconfig.h>
#include <isr.h>
#include <stdint.h>
#include <kmacros.h>
#include <alloc.h>

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

struct kmemnode;
struct heap;

void set_table_perms( unsigned int, unsigned long, page_dir_t * );
struct heap *init_heap( unsigned long start, unsigned long size, page_dir_t * );
unsigned long get_page( unsigned long, page_dir_t * );
unsigned long pop_page( void );
void push_page( unsigned long );
void set_page( unsigned long *address, unsigned long virtual, unsigned int permissions ); 

void map_pages( unsigned long start, unsigned long end, unsigned int permissions, page_dir_t * );
void free_pages( unsigned long start, unsigned long end, page_dir_t * );

void init_paging();
void set_page_dir( page_dir_t * );
void flush_tlb();

page_dir_t *clone_page_dir( page_dir_t *src );

#endif
