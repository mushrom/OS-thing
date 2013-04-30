#ifndef _kernel_alloc_h
#define _kernel_alloc_h
#include <paging.h>
#include <kmacros.h>

#define KHEAP_START  0xc0000000
#define KHEAP_SIZE   0x400000 
#define KHEAP_MAGIC  0xfee1dead
#define KHEAP_MIN_ALLOC 4
#define KHEAP_FREED  0xbadc0de
//#define KHEAP_SIZE  0x250000 
struct kmem_node;

typedef struct kmem_node {
	unsigned long magics;
} __attribute__((packed)) kmem_node_t;

typedef struct heap {
	unsigned long magics;
	unsigned long size; // This is the number of bytes total
	unsigned long used; // This is the number of bytes used
	struct kmem_node_t  *root;
	unsigned char full;
} __attribute__((packed)) heap_t;

struct heap *init_heap( unsigned long start, unsigned long size );

unsigned long kmalloc_e( unsigned long size, unsigned int align, unsigned long *physical );
unsigned long kmalloc_f( unsigned long size, unsigned int align, unsigned long *physical );
unsigned long kmalloc  ( unsigned long size, unsigned int align, unsigned long *physical );
unsigned long get_memused( void );

void expand( heap_t *heap );
void contract( heap_t *heap );
void kfree( void *ptr );

#endif
