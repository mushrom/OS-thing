#ifndef _kernel_lib_bmap_h
#define _kernel_lib_bmap_h
#include <alloc.h>
#include <kmacros.h>

#define BM_GET_BIT( m, i ) ((m[i/8] & (1 << (i % 8)))? 1: 0)
#define BM_SET_BIT( m, i, v ) (v?  (m[i/8] |=  (1 << (i % 8))):\
				   (m[i/8] &= ~(1 << (i % 8))))

char *create_bitmap( int size );
void free_bitmap( char *bitmap );

#endif
