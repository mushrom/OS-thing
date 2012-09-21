#ifndef _kernel_ordered_array_h
#define _kernel_ordered_array_h
#include <lib/stdint.h>
#include <lib/string.h>

typedef void* type_t;
typedef uint8_t (*lessthan_predicate_t)( type_t, type_t );
typedef struct {
	type_t *array;
	uint32_t size;
	uint32_t max_size;
	lessthan_predicate_t less_than;
} ordered_array_t;

#include <lib/kmacros.h>
#include <arch/x86/kheap.h>

uint8_t standard_lessthan_predicate( type_t a, type_t b );

ordered_array_t create_ordered_array( uint32_t, lessthan_predicate_t );
ordered_array_t place_ordered_array( void *, uint32_t, lessthan_predicate_t );
type_t lookup_ordered_array( uint32_t, ordered_array_t * );
void destroy_ordered_array( ordered_array_t * );
void insert_ordered_array( type_t, ordered_array_t * );
void remove_ordered_array( uint32_t, ordered_array_t * );

#endif
