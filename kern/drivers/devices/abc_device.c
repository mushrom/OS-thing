#ifndef _kernel_abc_device_c
#define _kernel_abc_device_c
#include <abc_device.h>

int meh_read( file_node_t *node, void *buf, unsigned long size );
int meh_pread( file_node_t *node, void *buf, unsigned long size, unsigned long offset );

int meh_read( file_node_t *node, void *buf, unsigned long size ){
	int i;
	 char *in = buf;
	for ( i = 0; i < size; i++ ){
		in[i] = 'a' + ( i % 26 );
	}
	return i;
}

int meh_pread( file_node_t *node, void *buf, unsigned long size, unsigned long offset ){
	int i;
	char *in = buf;
	for ( i = offset; i - offset < size; i++ ){
		in[i-offset] = 'a' + ( i % 26 );
	}
	return i-offset;
}

file_node_t *abc_create( ){
	file_node_t *ret = (file_node_t *)kmalloc( sizeof( file_node_t ), 0, 0 );

	memset( ret, 0, sizeof( file_node_t ));
	memcpy( ret->name, "abc0", 5 );
	ret->type = FS_CHAR_D;
	ret->find_node		= devfs_find_node;
	ret->read		= meh_read;
	ret->pread		= meh_pread;
	ret->mask 		= 0777;

	return ret;
}

#endif
