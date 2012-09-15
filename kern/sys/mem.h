#ifndef _kernel_memory_h
#define _kernel_memory_h
/*#include <sys/skio.h>*/
#include <sys/console.h>
#include <sys/skio.h>
#include <itoa.h>
/*#include <itoa.h>*/
#define KMEMROOT 0xf00000

void  init_kheap( void *start, unsigned int size );
void *kmalloc( unsigned long size );
void  kfree( void *ptr );
unsigned int get_memused( void );

#endif
