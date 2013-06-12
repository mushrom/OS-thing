#ifndef _kernel_memmap_h
#define _kernel_memmap_h
#include <paging.h>
#include <alloc.h>

enum {
	MEM_READ = 1,
	MEM_WRITE = 2,
	MEM_EXEC = 4,
	MEM_USER = 8
};

typedef struct memmap {
	unsigned long start;
	unsigned long end;
	unsigned char permissions;
	unsigned char present;
	unsigned long references;
	struct memmap *next;
} memmap_t;

memmap_t *memmap_create( unsigned long start, unsigned long end,
		unsigned char permissions, unsigned char present );
void memmap_delete( memmap_t *map );
int memmap_check( memmap_t *map, unsigned long address );
memmap_t *memmaps_check( memmap_t *map, unsigned long address );

#endif
