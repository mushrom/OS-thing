#ifndef _kernel_devfs_h
#define _kernel_devfs_h
#include <fs.h>

#define DEVFS_MAX_INODES 1024
struct file_node;
void init_devfs( void );
void devfs_register_device( struct file_node device );
struct file_node *devfs_find_node( struct file_node *node, char *name, unsigned int links ); 

#endif
