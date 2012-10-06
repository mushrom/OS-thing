#ifndef _kernel_devfs_h
#define _kernel_devfs_h
#include <fs/fs.h>

#define DEVFS_MAX_INODES 1024
void init_devfs( void );
void devfs_register_device( file_node_t device );
file_node_t *devfs_find_node( file_node_t *node, char *name ); 

#endif
