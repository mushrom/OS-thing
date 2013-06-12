#ifndef _kernel_devfs_c
#define _kernel_devfs_c
#include <devfs.h>

file_node_t 	**devfs_nodes;
file_node_t	*devfs_root;
file_node_t 	*temp;
extern file_node_t *fs_root;
unsigned long	devfs_i_count = 1;
struct dirent 	**devfs_dirents;

int devfs_open( file_node_t *node, char *path, int flags );
int devfs_close( file_node_t *node );

void init_devfs( void ){
	/*
	devfs_nodes = (void *)kmalloc( sizeof( file_node_t * ) * DEVFS_MAX_INODES, 0, 0 );
	devfs_nodes = (void *)kmalloc( sizeof( struct dirent ) * DEVFS_MAX_INODES, 0, 0 );
	devfs_root = devfs_nodes[0] = (void *)kmalloc( sizeof( file_node_t ), 0, 0 );
	memset( devfs_root, 0, sizeof( file_node_t ));
	
	memcpy( devfs_root->name, "devfs", 6 );
	devfs_root->type	= FS_DIR;
	devfs_root->mask	= 0777;
	//devfs_root->dirp	= (void *)kmalloc(sizeof( struct dirp ), 0, 0 );
	//memset( devfs_root->dirp, 0, sizeof( struct dirp ));
	devfs_root->inode	= 0;
	devfs_root->opendir	= vfs_opendir;
	devfs_root->closedir	= vfs_closedir;
	devfs_root->find_node	= devfs_find_node;
	devfs_root->open 	= devfs_open;
	devfs_root->close	= devfs_close;

	//if ( temp )
	//	temp->mount = devfs_root;

	temp = fs_find_node( fs_root, "dev", 1 );
	if (( fs_mount( devfs_root, temp, 0, 0 )) < 0 ){
		printf( "    Could not mount devfs\n" );
	} else {
		printf( "    Mounted devfs\n" );
	}
	*/

}

int devfs_open( file_node_t *node, char *path, int flags ){
	return 1;
}

int devfs_close( file_node_t *node ){
	return 0;
}

void devfs_register_device( file_node_t *device ){
	/*
	unsigned int index;
	file_node_t *new_device = device;

	new_device->inode = devfs_i_count++;
	new_device->find_node = devfs_find_node;
	devfs_nodes[new_device->inode] = new_device;

	index = devfs_root->dirp->dir_ptr++;
	devfs_root->dirp->dir[ index ] = (void *)kmalloc(sizeof( struct dirent ), 0, 0);

	memcpy( devfs_root->dirp->dir[index]->name, new_device->name, MAX_NAME_LEN );
	devfs_root->dirp->dir[ index ]->inode = new_device->inode;
	devfs_root->dirp->dir_count++;
	*/
}

file_node_t *devfs_find_node( file_node_t *node, char *name, unsigned int links ){ DEBUG_HERE 
	/*
	int i = 0, has_subdir = 0;
	file_node_t *ret = 0;
	char *sub_dir = 0;

	for ( i = 0; i < strlen( name ); i++ ){ DEBUG_HERE 
		if ( name[i] == '/' ){ DEBUG_HERE 
			has_subdir = 1;
			name[i] = 0;
			sub_dir = name + i + 1;
			break;
		}
	}
	for ( i = 0; i < node->dirp->dir_count; i++ ){ DEBUG_HERE 
		if ( strcmp((char *)node->dirp->dir[i]->name, name ) == 0 ){ DEBUG_HERE 
			ret = devfs_nodes[ node->dirp->dir[i]->inode ];
			if ( has_subdir ){ DEBUG_HERE 
				if ( node->find_node ){ DEBUG_HERE 
					return node->find_node( ret, sub_dir, 1 );
				} else { DEBUG_HERE 
					return 0;
				}
			} else { DEBUG_HERE 
				if ( ret->mount )
					return ret->mount;
				else
					return ret;
			}
		}
	}
	*/
	return 0;
}
#endif
