#ifndef _kernel_alloc_h
#define _kernel_alloc_h
#include <paging.h>
#include <kmacros.h>

#define KHEAP_START 0xc0000000
#define KHEAP_SIZE  0x100000   
#define KHEAP_MAGIC 0xfee1dead

typedef struct kmemnode {
	unsigned long magics;
	unsigned long size;
	struct kmemnode *next;
	struct kmemnode *prev;
} __attribute__((packed)) kmemnode_t;

typedef struct heap {
	kmemnode_t *memroot;
	kmemnode_t *memptr;
	kmemnode_t *memmove;
	unsigned long size;
	unsigned long max;
} __attribute__((packed)) heap_t;

unsigned long kmalloc_e( unsigned long size, unsigned int align, unsigned long *physical );
unsigned long kmalloc_f( unsigned long size, unsigned int align, unsigned long *physical );
unsigned long kmalloc  ( unsigned long size, unsigned int align, unsigned long *physical );
unsigned long get_memused( void );

void expand( unsigned long size, heap_t *heap );
void contract( unsigned long size, heap_t *heap );
void kfree( void *ptr );

#endif
