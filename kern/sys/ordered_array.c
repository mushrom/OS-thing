#ifndef _kernel_ordered_array_c
#define _kernel_ordered_array_c
#include <sys/ordered_array.h>

uint8_t standard_lessthan_predicate( type_t a, type_t b ){
	return ( a < b );
}

ordered_array_t create_ordered_array( uint32_t max_size, lessthan_predicate_t less_than ){
	ordered_array_t to_ret;
	to_ret.array = (void *)kmalloc( max_size * sizeof( type_t ), 0, 0 );
	memset( to_ret.array, 0, max_size * sizeof( type_t ));
	to_ret.size = 0;
	to_ret.max_size = max_size;
	to_ret.less_than = less_than;
	return to_ret;
}

ordered_array_t place_ordered_array( void *addr, uint32_t max_size, lessthan_predicate_t less_than ){
	ordered_array_t to_ret;
	to_ret.array = ( type_t *)addr;
	memset( to_ret.array, 0, max_size * sizeof( type_t ));
	to_ret.size = 0;
	to_ret.max_size = max_size;
	to_ret.less_than = less_than;

	return to_ret;
}

void destroy_ordered_array( ordered_array_t * array ){
}
void insert_ordered_array( type_t item, ordered_array_t *array ){
	uint32_t iterator = 0;
	type_t tmp, tmp2;

	assert( array->less_than, );
	while( iterator < array->size && array->less_than( array->array[iterator], item ))
		iterator++;
	if ( iterator == array->size ){
		array->array[array->size++] = item;
	} else {
		tmp = array->array[iterator];
		array->array[iterator] = item;
		while( iterator < array->size ){
			iterator++;
			tmp2 = array->array[iterator];
			array->array[iterator] = tmp;
			tmp = tmp2;
		}
		array->size++;
	}
}

type_t lookup_ordered_array( uint32_t i, ordered_array_t *array ){
	//printf( "l: i=0x%x, size=0x%x, ", i, array->size );
	assert( i < array->size, (type_t)0 );
	return array->array[i];
}

void remove_ordered_array( uint32_t i, ordered_array_t *array ){
	while ( i < array->size ){
		array->array[i] = array->array[i+1];
		i++;
	}
	array->size--;
}

#endif
