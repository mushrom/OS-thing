#ifndef _kernel_alloc_h
#define _kernel_alloc_h
#include <mem/paging.h>

#define KHEAP_START 0xc0000000
#define KHEAP_SIZE  0x100000   //That's 1M
#define KHEAP_MAGIC 0xcafebabe

typedef struct kmemnode {
	unsigned long magics;
	unsigned long size;
	struct kmemnode *next;
	struct kmemnode *prev;
} kmemnode_t;

typedef struct heap {
	kmemnode_t *kmemroot;
	kmemnode_t *kmemptr;
	kmemnode_t *kmemmove;
	unsigned long size;
	unsigned long max;
} heap_t;

unsigned long kmalloc_e( unsigned long size, unsigned int align, unsigned long *physical );
unsigned long kmalloc_f( unsigned long size, unsigned int align, unsigned long *physical );
unsigned long kmalloc  ( unsigned long size, unsigned int align, unsigned long *physical );

void expand( unsigned long size, heap_t *heap );
void contract( unsigned long size, heap_t *heap );

#endif
