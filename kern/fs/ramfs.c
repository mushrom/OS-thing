#ifndef _kernel_ramfs_c
#define _kernel_ramfs_c
#include <ramfs.h>

vfs_file_header_t	files[ MAX_INODES ];
unsigned long vfs_i_count = 0;

int vfs_mkdir( file_node_t *, char *, int );
int vfs_open ( file_node_t *, char *, int );
file_node_t *vfs_find_node( file_node_t *node, char *name, unsigned int links );
int vfs_read( file_node_t *node, void *buf, unsigned long size );
int vfs_pread( file_node_t *node, void *buf, unsigned long size, unsigned long offset );
int vfs_write( file_node_t *node, void *buf, unsigned long size );

file_node_t *vfs_root;

file_node_t *init_vfs( void ){ DEBUG_HERE
	vfs_root = (void *)kmalloc( sizeof( file_node_t ) * MAX_INODES, 0, 0 );
	memset( vfs_root, 0, sizeof( file_node_t ) * MAX_INODES );

	memcpy( vfs_root->name, "root", 5 );
	vfs_root->type 	= FS_DIR;
	vfs_root->mask 	= 0777;
	vfs_root->dirp	= (void *)kmalloc( sizeof( struct dirp ), 0, 0 );
	memset( vfs_root->dirp, 0, sizeof( struct dirp ));
	vfs_root->inode	= vfs_i_count++;

	vfs_root->opendir  = vfs_opendir;
	vfs_root->closedir = vfs_closedir;
	vfs_root->mkdir	  = vfs_mkdir;
	vfs_root->open	  = vfs_open;

	vfs_root->find_node= vfs_find_node;

	/*
	fs_opendir( vfs_root );
	fs_mkdir( vfs_root, "dev", 0777 );
	//open( vfs_root, "afile", 0777 );
	fs_mkdir( &vfs_root[1], "asubdir", 0777 );
	fs_mkdir( vfs_root, "init", 0777 );
	fs_open( vfs_root, "afile", 0777 );
	fs_closedir( vfs_root );
	*/

	return vfs_root;
}

file_node_t *vfs_get_node( int inode ){
	return &vfs_root[ inode ];
}

vfs_file_header_t *vfs_get_file_h( int inode ){
	return &files[ inode ];
}
	

file_node_t *vfs_find_node( file_node_t *node, char *name, unsigned int links ){ DEBUG_HERE
	int i = 0, has_subdir = 0;
	file_node_t *ret = 0, *temp = node;
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
			ret = vfs_get_node( node->dirp->dir[i]->inode );
			temp = ret;
			if ( ret->mount ){
				temp = ret->mount;
				//ret = temp;
			}
			if ( has_subdir ){ DEBUG_HERE
				if ( temp->find_node ){ DEBUG_HERE
					return temp->find_node( temp, sub_dir, links );
				} else { DEBUG_HERE
					return 0;
				}
			} else { DEBUG_HERE
				if ( links ){
					return temp;
				} else {
					return ret;
				}
			}
		}
	}
	return 0;
}

/* vfs functions, for vfs nodes */

int vfs_read( file_node_t *node, void *buf, unsigned long size ){ DEBUG_HERE
	char *output	= buf;
	char *file_data = vfs_get_file_h( node->inode )->data;//files[ node->inode ].data;
	int  file_len  = node->size, i;

	for ( i = 0; i < size && i < file_len; i++ ){
		output[i] = file_data[i];
	}

	return i;
}

int vfs_pread( file_node_t *node, void *buf, unsigned long size, unsigned long offset ){ DEBUG_HERE
	char *output	= buf;
	char *file_data = files[ node->inode ].data;
	int  file_len  = node->size, i;

	for ( i = offset; i < size && i < file_len; i++ ){
		output[i-offset] = file_data[i];
	}

	return i - offset;
}

int vfs_write( file_node_t *node, void *buf, unsigned long size ){ DEBUG_HERE
	char *input	= buf;
	char *file_data = files[ node->inode ].data;
	int  file_len  = node->size, i;
	//printf( "Writing to inode %d, length %d...\n", node->inode, node->size );

	if ( size > file_len ){
		file_data = (void *)kmalloc( size, 0, 0 );
		files[ node->inode ].data = file_data;
	}
	for ( i = 0; i < size; i++ ){
		file_data[i] = input[i];
	}
	node->size = size;
	//printf( "Wrote to inode %d, length %d...\n", node->inode, node->size );
	return i;
}

int vfs_pwrite( file_node_t *node, void *buf, unsigned long size, unsigned long offset ){ DEBUG_HERE
	char *input	= buf;
	char *file_data = files[ node->inode ].data;
	int  file_len  = node->size, i;
	//printf( "Writing to inode %d, length %d...\n", node->inode, node->size );

	if ( size > file_len ){
		file_data = (void *)kmalloc( size + offset + 1, 0, 0 );
		//memcpy( new_data, file_data, node->size );
		files[ node->inode ].data = file_data;
	}
	for ( i = offset; i < size + offset + 1; i++ ){
		file_data[i] = input[i - offset];
	}
	node->size = size;
	//printf( "Wrote to inode %d, length %d...\n", node->inode, node->size );
	return i;
}

int vfs_mkdir( file_node_t *node, char *name, int mode ){ DEBUG_HERE
	if ( node->dirp->dir_ptr <= node->dirp->dir_count ){ DEBUG_HERE
		unsigned long index = node->dirp->dir_ptr++;

		node->dirp->dir[ index ] = (void *)kmalloc( sizeof( struct dirent ), 0, 0 );
		vfs_root[ vfs_i_count ].dirp  = (void *)kmalloc( sizeof( struct dirp ),   0, 0 );
		memset( vfs_root[vfs_i_count].dirp, 0, sizeof( struct dirp ));
		vfs_root[ vfs_i_count ].opendir 		= vfs_opendir;
		vfs_root[ vfs_i_count ].mkdir		= vfs_mkdir;
		vfs_root[ vfs_i_count ].find_node 	= vfs_find_node;
		vfs_root[ vfs_i_count ].open		= vfs_open;
		vfs_root[ vfs_i_count ].mask		= mode;

		memcpy( node->dirp->dir[ index ]->name, name, strlen( name ) + 1);
		memcpy( vfs_root[ vfs_i_count ].name, name, strlen( name ) + 1);

		node->dirp->dir[ index ]->inode = vfs_i_count;
		vfs_root[ vfs_i_count ].inode = vfs_i_count;

		vfs_root[ vfs_i_count ].type = FS_DIR;

		node->dirp->dir_count++;
		vfs_i_count++;
		return 0;
	} else { DEBUG_HERE
		return -1;
	}
}

int vfs_open( file_node_t *node, char *name, int mode ){ DEBUG_HERE
	//printf( "Got here\n" );
	if ( node->type == FS_DIR ){ DEBUG_HERE
		int i = 0;
		for ( i = 0; i < node->dirp->dir_count; i++ ){
			if ( strcmp((char *)name, (char *)node->dirp->dir[i]->name ) == 0 ){
				return 0;
			}
		}
		if ( mode & O_CREAT ){
			unsigned long index = node->dirp->dir_ptr++;
			node->dirp->dir[ index ] = (void *)kmalloc( sizeof( struct dirent ), 0, 0 );
			memset( node->dirp->dir[index], 0, sizeof( struct dirent ));
			node->dirp->dir[ index ]->inode = vfs_i_count;
			memcpy( node->dirp->dir[ index ]->name, name, strlen( name ) + 1);
			memcpy( vfs_root[ vfs_i_count ].name, name, strlen( name ) + 1);

			vfs_root[ vfs_i_count ].inode 	= vfs_i_count;
			vfs_root[ vfs_i_count ].type 	= FS_FILE;
			vfs_root[ vfs_i_count ].read 	= vfs_read;
			vfs_root[ vfs_i_count ].pread 	= vfs_pread;
			vfs_root[ vfs_i_count ].write 	= vfs_write;
			vfs_root[ vfs_i_count ].pwrite 	= vfs_pwrite;
			vfs_root[ vfs_i_count ].mask	= 0777;

			node->dirp->dir_count++;
			vfs_i_count++;
			return 0;
		} else {
			return -1;
		}
	}
	return -1;
}

#endif
