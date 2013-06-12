#ifndef _kernel_ramfs_c
#define _kernel_ramfs_c
#include <ramfs.h>

//vfs_file_header_t	vfs_files[ MAX_INODES ];
vfs_file_header_t	*vfs_files;
unsigned long vfs_i_count = 0;

int vfs_mkdir( file_node_t *, char *, int );
int vfs_open ( file_node_t *, char *, int );
file_node_t *vfs_find_node( file_node_t *node, char *name, unsigned int links );
int vfs_read( file_node_t *node, void *buf, unsigned long size );
int vfs_pread( file_node_t *node, void *buf, unsigned long size, unsigned long offset );
int vfs_write( file_node_t *node, void *buf, unsigned long size );
int vfs_mknod( file_node_t *node, char *path, int mode, int dev );

file_node_t *vfs_root;

file_node_t *init_vfs( void ){ DEBUG_HERE
	vfs_files = (void *)kmalloc( sizeof( vfs_file_header_t ) * MAX_INODES, 0, 0 );
	vfs_root = (void *)kmalloc( sizeof( file_node_t ) * MAX_INODES, 0, 0 );
	memset( vfs_root, 0, sizeof( file_node_t ) * MAX_INODES );

	memcpy( vfs_root->name, "root", 5 );
	vfs_root->type 	= FS_DIR;
	vfs_root->mask 	= 0777;
	//vfs_root->dirp	= (void *)kmalloc( sizeof( struct dirp ), 0, 0 );
	//memset( vfs_root->dirp, 0, sizeof( struct dirp ));
	vfs_root->inode	= vfs_i_count++;

	vfs_files[0].data = 0;
	/*
	vfs_root->opendir  = vfs_opendir;
	vfs_root->closedir = vfs_closedir;
	*/
	vfs_root->read    = vfs_read;
	vfs_root->pread   = vfs_pread;
	vfs_root->mkdir	  = vfs_mkdir;
	vfs_root->open	  = vfs_open;
	vfs_root->size    = 0;

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
	return &vfs_files[ inode ];
}
	

file_node_t *vfs_find_node( file_node_t *node, char *name, unsigned int links ){ DEBUG_HERE
	int i = 0, has_subdir = 0;
	file_node_t *ret = 0, *temp = node;
	char *sub_dir = 0, *file_data;
	struct dirent *dir;

	for ( i = 0; i < strlen( name ); i++ ){ DEBUG_HERE
		if ( name[i] == '/' ){ DEBUG_HERE
			has_subdir = 1;
			name[i] = 0;
			sub_dir = name + i + 1;
			break;
		}
	}

	//printf( "hello from vfs_find_node. \n" );

	file_data = (char *)(vfs_get_file_h( node->inode )->data);
	dir = (struct dirent *)file_data;
	int dir_size = node->size / sizeof( struct dirent );
	for ( i = 0; i < dir_size; i++ ){ DEBUG_HERE
		//printf( "[fn] Comparing 0x%x:\"%s\" to 0x%x:\"%s\"\n", dir, dir[i].name, 
		//	name, name );
		if ( strcmp( name, "."  ) == 0 ){
			if ( has_subdir ){ DEBUG_HERE
				if ( temp->find_node ){ DEBUG_HERE
					return node->find_node( node, sub_dir, links );
				} else { DEBUG_HERE
					return 0;
				}
			} else { DEBUG_HERE
				return node;
			}
		} else if ( strcmp( dir[i].name, name ) == 0 ){ DEBUG_HERE
			ret = vfs_get_node( dir[i].inode );
			temp = ret;
			if ( ret->mount ){
				temp = ret->mount;
				//printf( "Is mounted...\n" );
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
	//printf( "Reading \"%s\":%d:%d... ", node->name, offset, size );
	char *output	= buf;
	char *file_data = vfs_files[ node->inode ].data;
	int  file_len  = node->size, i, j;

	for ( i = offset, j = 0; j < size && i < node->size; j++, i++ ){
		output[j] = file_data[i];
	}

	//printf( "returning %d\n", i - offset );
	return i - offset;
}

int vfs_write( file_node_t *node, void *buf, unsigned long size ){ DEBUG_HERE
	char *input	= buf;
	char *file_data = vfs_files[ node->inode ].data;
	int  file_len  = node->size, i;
	//printf( "Writing to inode %d, length %d...\n", node->inode, node->size );

	if ( size > file_len ){
		if ( file_data )
			kfree( file_data );

		file_data = (void *)kmalloc( size, 0, 0 );
		vfs_files[ node->inode ].data = file_data;
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
	char *file_data = vfs_files[ node->inode ].data;
	int  file_len  = node->size, i;
	//printf( "Writing to inode %d, length %d...\n", node->inode, node->size );

	if ( size > file_len ){
		//kfree( file_data ); // Enable this after debugging
		file_data = (void *)kmalloc( size + offset + 1, 0, 0 );
		//memcpy( new_data, file_data, node->size );
		vfs_files[ node->inode ].data = file_data;
	}
	for ( i = offset; i < size + offset + 1; i++ ){
		file_data[i] = input[i - offset];
	}
	node->size = size;
	//printf( "Wrote to inode %d, length %d...\n", node->inode, node->size );
	return i;
}

int vfs_mkdir( file_node_t *node, char *name, int mode ){ DEBUG_HERE
	struct dirent *dir;
	char *file_data = vfs_get_file_h( node->inode )->data;

	//unsigned long index = node->dirp->dir_ptr++;
	//file_node_t *new_dir;
	vfs_file_header_t *nodehead;

	dir = (struct dirent *)kmalloc( node->size + sizeof( struct dirent ), 0, 0);
	if ( file_data ){
		memcpy( dir, file_data, node->size );
		kfree( file_data );
	}
	file_data = vfs_files[ node->inode ].data = (char *)dir;
	dir = (struct dirent *)((unsigned long)dir + node->size);
	//printf( "file_data: 0x%x, dir: 0x%x, node->size: 0x%x\n", file_data, dir, node->size );

	nodehead = vfs_get_file_h( vfs_i_count );
	nodehead->data = (vfs_file_header_t *)kmalloc( sizeof( struct dirent ), 0, 0 );

	vfs_root[ vfs_i_count ].mkdir		= vfs_mkdir;
	vfs_root[ vfs_i_count ].find_node 	= vfs_find_node;
	vfs_root[ vfs_i_count ].read		= vfs_read;
	vfs_root[ vfs_i_count ].pread		= vfs_pread;
	vfs_root[ vfs_i_count ].open		= vfs_open;
	vfs_root[ vfs_i_count ].mknod		= vfs_mknod;
	vfs_root[ vfs_i_count ].mask		= mode;
	vfs_root[ vfs_i_count ].inode 		= vfs_i_count;
	vfs_root[ vfs_i_count ].type 		= FS_DIR;

	memcpy( dir->name, name, MAX_NAME_LEN );
	memcpy( vfs_root[ vfs_i_count ].name, name, MAX_NAME_LEN );

	//printf( "added \"%s\" to dir\n", dir->name );

	dir->inode = vfs_i_count;

	node->size += sizeof( struct dirent );
	vfs_i_count++;
	return 0;

	/*
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
	*/
}

int vfs_mknod( file_node_t *node, char *name, int mode, int dev ){ DEBUG_HERE
	if ( node->type != FS_DIR )
		return -1;

	//printf( "Making \"%s\"\n", name );
	/* Move this to mknod */
	struct dirent *dir = vfs_get_file_h( node->inode )->data;
	char *file_data = vfs_get_file_h( node->inode )->data;
	//file_node_t *new_dir;
	vfs_file_header_t *nodehead;

	dir = (struct dirent *)kmalloc( node->size + sizeof( struct dirent ), 0, 0);
	if ( file_data ){
		memcpy( dir, file_data, node->size );
		kfree( file_data );
	}
	file_data = vfs_files[ node->inode ].data = (char *)dir;
	dir = (struct dirent *)((unsigned long)dir + node->size);

	/*
	dir = (struct dirent *)kmalloc( node->size + sizeof( struct dirent ), 0, 0);
	if ( file_data )
		memcpy( dir, file_data, node->size );
		file_data = 
		kfree( file_data );
	}
	dir = (struct dirent *)((unsigned long)dir + node->size);
	*/

	nodehead = vfs_get_file_h( vfs_i_count );
	//nodehead->data = (vfs_file_header_t *)kmalloc( sizeof( struct dirent ), 0, 0 );
	nodehead->data = 0;

	vfs_root[ vfs_i_count ].find_node 	= vfs_find_node;
	vfs_root[ vfs_i_count ].open		= vfs_open;
	vfs_root[ vfs_i_count ].read		= vfs_read;
	vfs_root[ vfs_i_count ].pread		= vfs_pread;
	vfs_root[ vfs_i_count ].write		= vfs_write;
	vfs_root[ vfs_i_count ].pwrite		= vfs_pwrite;
	vfs_root[ vfs_i_count ].mknod		= vfs_mknod;

	vfs_root[ vfs_i_count ].mask		= mode;
	vfs_root[ vfs_i_count ].inode 		= vfs_i_count;
	vfs_root[ vfs_i_count ].type 		= dev;
	vfs_root[ vfs_i_count ].size 		= 0;

	memcpy( dir->name, name, MAX_NAME_LEN );
	memcpy( vfs_root[ vfs_i_count ].name, name, MAX_NAME_LEN );

	dir->inode = vfs_i_count;

	node->size += sizeof( struct dirent );
	vfs_i_count++;
	return 0;
}

int vfs_open( file_node_t *node, char *name, int mode ){ DEBUG_HERE
	//printf( "got here, to vfs_open, trying \"%s\":0x%x\n", node->name, node->size );
	//printf( "Got here\n" );
	if ( node->type == FS_DIR && mode & O_CREAT ){
		return vfs_mknod( node, name, 0777, FS_FILE );
	}

	return node->inode;
}

#endif
