#ifndef _user_alloc_c
#define _user_alloc_c

#include <stdio.h>
#include <syscall.h>
#include <alloc.h>

#define MAGIC 0xdeadbabe
#define FREED 0xfee1dead

unsigned long h_size = 0;
unsigned long h_pos  = 0;
unsigned long t_size = 0;

typedef struct mnode {
	unsigned magics;
} mnode_t;

void resize(){
	if ( !h_pos )
		h_pos = (unsigned long)sbrk( 0 );

	if ( h_size ){
		sbrk( h_size );
		h_size *= 2;
	} else {
		sbrk( 8 );
		h_size = 8;
	}
}

unsigned long find_free( mnode_t *node, unsigned node_size, unsigned size ){
	unsigned int s  = node_size / 2;
	unsigned int ret;

	mnode_t *temp = (void *)((unsigned long)node + (unsigned long)s);

	if ( s < sizeof( mnode_t * ) || ( temp->magics == MAGIC && node->magics == MAGIC )) 
		return 0;

	if ( size < s / 2 ){
		if (	( ret = find_free( temp, s, size )) ||
			( ret = find_free( node, s, size )))
			return ret;
	}

	if ( temp->magics != MAGIC ) {
		temp->magics = MAGIC;
		return (unsigned long)temp + 4;
	} else if ( node->magics != MAGIC ){
		node->magics = MAGIC;
		return (unsigned long)node + 4;
	}

	return 0;
}

void *malloc( int size ){
	unsigned long ret = 0;
	t_size += size + 8;

	while ( size + sizeof( mnode_t ) > h_size / 2 )
		resize();

	while (( ret = find_free((mnode_t *)h_pos, h_size, size + sizeof( mnode_t ))) == 0 )
		resize();

	return (void *)ret;
}

void free( void *ptr ){
	mnode_t *p = ptr - 4;

	if ( p->magics == MAGIC ){
		p->magics = FREED;
	} else {
		printf( "Warning: could not free pointer 0x%x: ", p );
		if ( p->magics == FREED )
			printf( "Likely a double-freed pointer." );
		printf( "\n" );
		exit(1);
	}
}

#endif
