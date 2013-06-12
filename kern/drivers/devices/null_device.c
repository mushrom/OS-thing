#ifndef _kernel_null_device_c
#define _kernel_null_device_c
#include <null_device.h>

int null_read( file_node_t *node, void *buf, unsigned long size );
int null_pread( file_node_t *node, void *buf, unsigned long size, unsigned long offset );
int null_write( file_node_t *node, void *buf, unsigned long size );
int null_open( file_node_t *node, char *path, int flags );
int null_close( file_node_t *node );

int null_read( file_node_t *node, void *buf, unsigned long size ){
	return 0;
}

int null_pread( file_node_t *node, void *buf, unsigned long size, unsigned long offset ){
	return 0;
}

int null_write( file_node_t *node, void *buf, unsigned long size ){
	return size;
}

int null_open( file_node_t *node, char *path, int flags ){
	return 1;
}

int null_close( file_node_t *node ){
	return 0;
}

file_node_t *null_create( ){
	file_node_t *ret = (file_node_t *)kmalloc( sizeof( file_node_t ), 0, 0 );

	memset( ret, 0, sizeof( file_node_t ));
	memcpy( ret->name, "null", 5 );
	ret->type 		= FS_CHAR_D;
	ret->find_node		= devfs_find_node;
	ret->read		= null_read;
	ret->pread		= null_pread;
	ret->write		= null_write;
	ret->open		= null_open;
	ret->close		= null_close;
	ret->mask 		= 0777;

	return ret;
}



#endif
