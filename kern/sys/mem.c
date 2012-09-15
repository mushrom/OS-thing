#ifndef _kernel_memory_c
#define _kernel_memory_c
#include <sys/mem.h>
//#include <sys/skio.h>
/*
#include <sys/console.h>
#include <itoa.h>*/

typedef struct kmemnode {
	unsigned int 	check,
			size;
	struct kmemnode *prev;
	struct kmemnode *next;
} kmemnode_t;

/*void  init_kheap( void *start, unsigned int size );
void *kmalloc( unsigned long size );
void  kfree( void *ptr );
*/

unsigned int	 heap_size   = 0x100000,
		 block_size  = sizeof( kmemnode_t ),
		 memused     = 0;
kmemnode_t	*kmemroot    = (kmemnode_t *)KMEMROOT, 
		*kmemptr     = (kmemnode_t *)KMEMROOT; 

void init_kheap( void *start, unsigned int size ){
	unsigned int i = 0;
	heap_size = size;
	kmemroot = start;
	kmemptr = kmemroot;
	for ( i = 0; i < size; i += block_size ){
		kmemptr->next = kmemptr + block_size;
		kmemptr->prev = kmemptr - block_size;
		kmemptr->size = 0;
		kmemptr->check = 0;
		//kputs( "Initialized block " ); print_num( i ); kputs( ": \t\t0x" ); print_hex(( unsigned long )kmemptr );
		kmemptr = kmemptr->next;
		//kputs( "->0x" ); print_hex(( unsigned long )kmemptr ); kputs( "\n" );
	}
  	kputs( "Initialized heap... ( 0x" ); print_hex( heap_size ); kputs( " bytes, " );
		print_num( heap_size / block_size ); kputs( " blocks )\n" );
	kputs( "Block size: " ); print_num( block_size ); kputs( "\n" );
}

void *kmalloc( unsigned long size ){
	unsigned int 	found_free = 0,
			i = 0;
	kmemnode_t *kmemmove;
	kmemnode_t *ret;
	kmemptr = kmemroot;
	while ( !found_free ){
		if ( kmemptr->size == 0 ){
			kmemmove = kmemptr;
			for ( i = 0; i < size; ){
				if ( kmemmove->next->size == 0 ){
					i += block_size;
					kmemmove = kmemmove->next;
				} else {
					i = 0;
					break;
				}
			}
			if ( i ){
				ret = kmemptr->next;
				kputs( "0x" ); print_hex(( unsigned long )kmemptr->next );
				kmemptr->next = kmemmove->next;
				kputs( ":0x" ); print_hex(( unsigned long )kmemptr->next );
				kputs( ":0x" ); print_hex(( unsigned long )ret );
				kputs( "\n" );
				kmemptr->next->prev = kmemptr;
				kmemptr->size = size;
				found_free = 1;
				break;
			}
		} else {
			kmemptr = kmemptr->next;
		}
	}
	memused += i;
	return ret;
}

void kfree( void *ptr ){
	unsigned long size;
	kmemnode_t *kmemmove = kmemptr = kmemroot;

	kmemptr = ptr - block_size;
	print_num( kmemptr->size ); kputs( "\n" );
}

unsigned int get_memused( void ){
	return memused;
}

/*void *kmalloc( unsigned long size ){
	char *ret = kmemkmemroot;
	kmemptr += size;

	return ret;
}*/

/*unsigned int memused( void ){
	return kheap_p - kheap_start;
}*/

#endif
