#ifndef _kernel_fs_c
#define _kernel_fs_c
#include <fs/fs.h>

file_node_t 	*fs_root;
unsigned long vfs_i_count = 0;
vfs_file_header_t	files[ MAX_INODES ];

int vfs_mkdir( file_node_t *, char *, int );
int vfs_open ( file_node_t *, char *, int );
file_node_t *vfs_find_node( file_node_t *node, char *name );
int vfs_read( file_node_t *node, void *buf, unsigned long size );
int vfs_write( file_node_t *node, void *buf, unsigned long size );

void init_vfs( void ){ DEBUG_HERE
	fs_root = (void *)kmalloc( sizeof( file_node_t ) * MAX_INODES, 0, 0 );
	memset( fs_root, 0, sizeof( file_node_t ) * MAX_INODES );

	memcpy( fs_root->name, "root", 5 );
	fs_root->type 	= FS_DIR;
	fs_root->mask 	= 0777;
	fs_root->dirp	= (void *)kmalloc( sizeof( struct dirp ), 0, 0 );
	memset( fs_root->dirp, 0, sizeof( struct dirp ));
	fs_root->inode	= vfs_i_count++;

	fs_root->opendir  = vfs_opendir;
	fs_root->closedir = vfs_closedir;
	fs_root->mkdir	  = vfs_mkdir;
	fs_root->open	  = vfs_open;

	fs_root->find_node= vfs_find_node;

	fs_opendir( fs_root );
	fs_mkdir( fs_root, "dev", 0777 );
	//open( fs_root, "afile", 0777 );
	fs_mkdir( &fs_root[1], "asubdir", 0777 );
	fs_mkdir( fs_root, "test", 0777 );
	fs_open( fs_root, "afile", 0777 );
	fs_closedir( fs_root );
}

file_node_t *vfs_find_node( file_node_t *node, char *name ){ DEBUG_HERE
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
			ret = &fs_root[ node->dirp->dir[i]->inode ];
			if ( has_subdir ){ DEBUG_HERE
				if ( node->find_node ){ DEBUG_HERE
					return node->find_node( ret, sub_dir );
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
	return 0;
}

/* vfs functions, for vfs nodes */
struct dirp *vfs_opendir( file_node_t *node ){ DEBUG_HERE
	node->dirp->dir_ptr = 0;
	node->links++;
	return node->dirp;
}

int vfs_closedir( file_node_t *node ){ DEBUG_HERE
	node->links--;
	return 0;
}

int vfs_read( file_node_t *node, void *buf, unsigned long size ){ DEBUG_HERE
	char *output	= buf;
	char *file_data = files[ node->inode ].data;
	int  file_len  = node->size, i;

	for ( i = 0; i < size && i < file_len; i++ ){
		output[i] = file_data[i];
	}

	return i;
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

int vfs_mkdir( file_node_t *node, char *name, int mode ){ DEBUG_HERE
	if ( node->dirp->dir_ptr <= node->dirp->dir_count ){ DEBUG_HERE
		unsigned long index = node->dirp->dir_ptr++;

		node->dirp->dir[ index ] = (void *)kmalloc( sizeof( struct dirent ), 0, 0 );
		fs_root[ vfs_i_count ].dirp  = (void *)kmalloc( sizeof( struct dirp ),   0, 0 );
		memset( fs_root[vfs_i_count].dirp, 0, sizeof( struct dirp ));
		fs_root[ vfs_i_count ].opendir 		= vfs_opendir;
		fs_root[ vfs_i_count ].closedir 	= vfs_closedir;
		fs_root[ vfs_i_count ].mkdir		= vfs_mkdir;
		fs_root[ vfs_i_count ].find_node 	= vfs_find_node;
		fs_root[ vfs_i_count ].open		= vfs_open;

		memcpy( node->dirp->dir[ index ]->name, name, strlen( name ));
		memcpy( fs_root[ vfs_i_count ].name, name, strlen( name ));

		node->dirp->dir[ index ]->inode = vfs_i_count;
		fs_root[ vfs_i_count ].inode = vfs_i_count;

		fs_root[ vfs_i_count ].type = FS_DIR;

		node->dirp->dir_count++;
		vfs_i_count++;
		return 0;
	} else { DEBUG_HERE
		return -1;
	}
}

int vfs_open( file_node_t *node, char *name, int mode ){ DEBUG_HERE
	if ( node->type == FS_DIR ){ DEBUG_HERE
		int i = 0;
		for ( i = 0; i < node->dirp->dir_count; i++ ){
			if ( strcmp((char *)name, (char *)node->dirp->dir[i]->name ) == 0 ){
				return 0;
			}
		}
		unsigned long index = node->dirp->dir_ptr++;
		node->dirp->dir[ index ] = (void *)kmalloc( sizeof( struct dirent ), 0, 0 );
		memset( node->dirp->dir[index], 0, sizeof( struct dirent ));
		node->dirp->dir[ index ]->inode = vfs_i_count;
		memcpy( node->dirp->dir[ index ]->name, name, strlen( name ));
		memcpy( fs_root[ vfs_i_count ].name, name, strlen( name ));

		fs_root[ vfs_i_count ].inode 	= vfs_i_count;
		fs_root[ vfs_i_count ].type 	= FS_FILE;
		fs_root[ vfs_i_count ].read 	= vfs_read;
		fs_root[ vfs_i_count ].write 	= vfs_write;

		node->dirp->dir_count++;
		vfs_i_count++;
	}
	return 0;
}


/* A ton of wrapper functions... All they do is check if a function pointer is set,
 * return an error if not, otherwise return the result of the function. 			 */
int  fs_write( file_node_t *node, void *buf, unsigned long size ){ DEBUG_HERE
	if ( node->write )
		return node->write( node, buf, size );
	else
		return -1;
}

int  fs_read ( file_node_t *node, void *buf, unsigned long size ){ DEBUG_HERE
	if ( node->read )
		return node->read( node, buf, size );
	else 
		return -1;
}

int fs_pwrite( file_node_t *node, void *buf, unsigned long size, unsigned long offset ){ DEBUG_HERE
	if ( node->pwrite )
		return node->pwrite( node, buf, size, offset );
	else
		return -1;
}

int  fs_pread( file_node_t *node, void *buf, unsigned long size, unsigned long offset ){ DEBUG_HERE
	if ( node->pread )
		return node->pread( node, buf, size, offset );
	else 
		return -1;
}

int   fs_open( file_node_t *node, char *name, int i ){ DEBUG_HERE
	if ( node->open )
		return node->open( node, name, i );
	else
		return -1;
}

int  fs_close( file_node_t *node ){ DEBUG_HERE
	if ( node->close )
		return node->close( node );
	else 
		return -1;
}

struct dirp *fs_opendir( file_node_t *node ){ DEBUG_HERE
	if ( node->opendir )
		return node->opendir( node );
	else
		return 0;
}

struct dirent *fs_readdir( struct dirp *dir ){ DEBUG_HERE
	if ( !dir || dir->dir_ptr >= dir->dir_count ){ DEBUG_HERE
		return 0;
	} else { DEBUG_HERE
		return dir->dir[ dir->dir_ptr++ ];
	}
}

int fs_closedir( file_node_t *node ){ DEBUG_HERE
	if ( node->closedir )
		return node->closedir( node );
	else 
		return -1;
}

int fs_mkdir ( file_node_t *node, char *name, unsigned long mode ){ DEBUG_HERE
	if ( node->dirp && node->mkdir ){ DEBUG_HERE
		return node->mkdir( node, name, mode );
	} else { DEBUG_HERE
		return -1;
	}
}

int fs_stat ( file_node_t *node, struct vfs_stat *buf ){
	buf->type = node->type;
	buf->uid  = node->uid;
	buf->gid  = node->gid;
	buf->time = node->time;
	buf->size = node->size;
	buf->mask = node->mask;

	return 0;
}

file_node_t *fs_find_node( file_node_t *node, char *name ){ DEBUG_HERE
	if ( node->dirp && node->find_node ){ DEBUG_HERE
		return node->find_node( node, name );
	} else { DEBUG_HERE
		return 0;
	}
}
#endif
