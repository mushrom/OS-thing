#ifndef _kernel_lib_bmap_c
#define _kernel_lib_bmap_c
#include <bmap.h>

char *create_bitmap( int size ){
	return knew( char[ size / 8 + 1 ]);
}

void free_bitmap( char *bitmap ){
	kfree( bitmap );
}

#endif
