#ifndef _kernel_memory_h
#define _kernel_memory_h

#include <sys/console.h>
#include <lib/stdio.h>
#include <lib/stdint.h>
#include <arch/arch.h>
#include <sys/ordered_array.h>

#define KHEAP_START	0xc0000000
#define KHEAP_INIT_SIZE 0x100000
#define HEAP_INDEX_SIZE 0x20000
#define HEAP_MAGIC	0x123890ab
#define HEAP_MIN_SIZE	0x70000

typedef struct {
	uint32_t magic;
	uint8_t  is_hole;
	uint32_t size;
} header_t;

typedef struct {
	uint32_t magic;
	header_t *header;
} footer_t;

typedef struct {
	ordered_array_t index;
	uint32_t start_addr;
	uint32_t end_addr;
	uint32_t max_addr;
	uint8_t  supervisor;
	uint8_t  readonly;
} heap_t;

heap_t *create_heap( uint32_t, uint32_t, uint32_t, uint8_t, uint8_t );

void *alloc( uint32_t, uint8_t, heap_t * );
void free( void *p, heap_t *heap );

void *kmalloc( uint32_t, uint8_t, uint32_t * );
void  kfree( void * );
unsigned int get_memused( void );

#define kmalloc_a(  a )     kmalloc( a, 1, 0 )
#define kmalloc_p(  a, b )  kmalloc( a, 0, b )
#define kmalloc_ap( a, b )  kmalloc( a, 1, b )

#endif
