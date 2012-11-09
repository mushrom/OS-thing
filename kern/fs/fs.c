#ifndef _kernel_fs_c
#define _kernel_fs_c
#include <fs.h>

file_node_t 	*fs_root;
unsigned long vfs_i_count = 0;
vfs_file_header_t	files[ MAX_INODES ];

int vfs_mkdir( file_node_t *, char *, int );
int vfs_open ( file_node_t *, char *, int );
file_node_t *vfs_find_node( file_node_t *node, char *name, unsigned int links );
int vfs_read( file_node_t *node, void *buf, unsigned long size );
int vfs_pread( file_node_t *node, void *buf, unsigned long size, unsigned long offset );
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
	fs_mkdir( fs_root, "init", 0777 );
	fs_open( fs_root, "afile", 0777 );
	fs_closedir( fs_root );
}

file_node_t *fs_find_path( char *path, unsigned int links ){
	extern task_t *current_task;
	file_node_t *fp;
	if ( strlen( path ) == 0 )
		return 0;

	if ( path[0] == '/' ){
		fp = current_task->root;
		if ( strlen( path ) == 1 )
			return fp;
		path++;
	} else if ( path[0] == '.' ){
		fp = current_task->cwd;
		if ( strlen( path ) == 1 )
			return fp;
		path++;
	} else {
		fp = current_task->cwd;
	}
	return fs_find_node( fp, path, links );
}

int open( char *path, int flags ){
	extern task_t *current_task;
	file_node_t *fp = fs_find_path( path, 1 );
	unsigned long i = 0;
	//int ret;
	
/*
	for ( i = strlen( path ); i; i-- ){
		if ( path[i] == '/' ){
			path[i] = 0;
			break;
		}
	}
	ret = fs_open( current_task->cwd );
*/
	if ( !fp )
		return -1;
	if ( current_task->file_count >= MAX_FILES )
		return -1;
	
	for ( i = 0; i < MAX_FILES; i++ ){
		if ( current_task->files[i] == 0 )
			break;
	}
	
	current_task->files[i] = (void *)kmalloc( sizeof( file_descript_t ), 0, 0 );
	current_task->files[i]->file = fp;
	current_task->files[i]->r_offset = 0;
	current_task->files[i]->w_offset = 0;
	current_task->files[i]->d_offset = 0;
	current_task->file_count++;	
	
	return i;
}

int close( int fd ){
	extern task_t *current_task;

	if ( !current_task->file_count || !current_task->files[fd] )
		return -1;

	current_task->files[fd] = 0;
	current_task->file_count--;
	return 0;
}

int read( int fd, void *buf, unsigned long size ){
	extern task_t *current_task;
	//printf( "[vfs] %d, 0x%x, %d\n", fd, buf, size );
	if ( !current_task->file_count || fd >= current_task->file_count )
		return -1;

	if ( !current_task->files[fd] )
		exit_thread();
	
	unsigned long i = 0, offset = current_task->files[fd]->r_offset;
	i = fs_pread( current_task->files[fd]->file, buf, size, offset );
	//printf( "[vfs] %d:%d:%d\n", current_task->files[fd]->file->inode, fd, i );
	//printf( "%d: %d:%d\n", fd, offset, current_task->files[fd]->r_offset );
	current_task->files[fd]->r_offset += i;

	return i;
}

int write( int fd, void *buf, unsigned long size ){
	extern task_t *current_task;
	if ( !current_task->file_count || fd >= current_task->file_count )
		return -1;

	if ( !current_task->files[fd] )
		exit_thread();

	unsigned long i = 0, offset = current_task->files[fd]->w_offset;
	i = fs_pwrite( current_task->files[fd]->file, buf, size, offset );
	if ( i > 0 )
		current_task->files[fd]->w_offset += i;

	return i;
}

struct dirp *fdopendir( int fd ){
	extern task_t *current_task;
	if ( !current_task->file_count || fd >= current_task->file_count )
		return 0;

	if ( !current_task->files[fd] )
		exit_thread();

	if ( !current_task->files[fd]->file->dirp )
		return 0;

	current_task->files[fd]->d_offset = 0;
	
	return current_task->files[fd]->file->dirp;
}

struct dirent *readdir( int fd, struct dirp *dir ){
	extern task_t *current_task;
	if ( !current_task->file_count || fd >= current_task->file_count )
		return 0;

	if ( !current_task->files[fd] )
		exit_thread();

	if ( current_task->files[fd]->d_offset >= dir->dir_count || !dir )
		return 0;

	return dir->dir[ current_task->files[fd]->d_offset++ ];
}

int chdir( char *path ){
	extern task_t *current_task;

	file_node_t *fp = fs_find_path( path, 1 );
	if ( !fp )
		return -1;

	current_task->cwd = fp;

	return 0;
}

int chroot( char *path ){
	extern task_t *current_task;

	file_node_t *fp = fs_find_path( path, 1 );
	if ( !fp )
		return -1;

	current_task->root = fp;

	return 0;
}

int lseek( int fd, long offset, int whence ){
	extern task_t *current_task;
	int size;
	if ( !current_task->file_count || fd >= current_task->file_count )
		return -1;

	if ( !current_task->files[fd] )
		exit_thread();

	switch ( whence ){
		case 0:
			if ( offset < 0 )
				return -1;
			current_task->files[fd]->r_offset = offset;
			current_task->files[fd]->w_offset = offset;
			break;
		case 1:
			if ( current_task->files[fd]->r_offset + offset < 0 )
				return -1;
			current_task->files[fd]->r_offset += offset;
			current_task->files[fd]->w_offset += offset;
			break;
		case 2:
			size = current_task->files[fd]->file->size;
			if ( size + offset < 0 )
				return -1;
			current_task->files[fd]->r_offset = size + offset;
			current_task->files[fd]->w_offset = size + offset;
			break;
		default:
			return -1;
	}

	return current_task->files[fd]->r_offset;
}

int mkdir( char *path, int mode ){
	extern task_t *current_task;

	file_node_t *dir = fs_find_path( path, 1 );
	char *file = 0;
	int i;

	if ( dir ) /* If the path exists, return */
		return -1;

	for ( i = strlen( path ); i > 0; i-- ){ /* Find first directory above the directory to make */
		if ( path[i] == '/' ){
			path[i] = 0;
			file = path + i + 1;
			break;
		}
	}
	
	if ( i ){ /* Found a directory in the path, try and find the node */
		dir = fs_find_path( path, 1 );
	} else { /* No directory/was root, search current directory or root */
		file = path;
		if ( file[0] == '/' ){
			file++;
			dir = fs_find_path( "/", 1 );
		} else {
			dir = fs_find_path( ".", 1 );
		}
	}
	
	if ( !dir )
		return -1;

	return fs_mkdir( dir, file, mode );
}

int mount( char *type, char *dir, int flags, void *data ){
	file_node_t *type_fp, *dir_fp;

	type_fp = fs_find_path( type, 1 );
	dir_fp  = fs_find_path( dir, 1 );

	if ( !dir_fp )
		return -1;

	if ( !type_fp ){
		type_fp = fs_find_node( fs_root, type, 1 );
		if ( !type_fp )
			return -1;
	}

	return fs_mount( type_fp, dir_fp, flags, data );
}

int unmount( char *dir, int flags ){
	file_node_t *dir_fp;

	dir_fp  = fs_find_path( dir, 0 );
	if ( !dir_fp )
		return -1;

	return fs_unmount( dir_fp, flags );
}

int fs_mount( file_node_t *type, file_node_t *dir, int flags, void *data ){
	dir->mount = type;

	return 0;
}

int fs_unmount( file_node_t *dir, int flags ){
	dir->mount = 0;

	return 0;
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
			ret = &fs_root[ node->dirp->dir[i]->inode ];
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
		fs_root[ vfs_i_count ].dirp  = (void *)kmalloc( sizeof( struct dirp ),   0, 0 );
		memset( fs_root[vfs_i_count].dirp, 0, sizeof( struct dirp ));
		fs_root[ vfs_i_count ].opendir 		= vfs_opendir;
		fs_root[ vfs_i_count ].mkdir		= vfs_mkdir;
		fs_root[ vfs_i_count ].find_node 	= vfs_find_node;
		fs_root[ vfs_i_count ].open		= vfs_open;

		memcpy( node->dirp->dir[ index ]->name, name, strlen( name ) + 1);
		memcpy( fs_root[ vfs_i_count ].name, name, strlen( name ) + 1);

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
		if ( mode & O_CREAT ){
			unsigned long index = node->dirp->dir_ptr++;
			node->dirp->dir[ index ] = (void *)kmalloc( sizeof( struct dirent ), 0, 0 );
			memset( node->dirp->dir[index], 0, sizeof( struct dirent ));
			node->dirp->dir[ index ]->inode = vfs_i_count;
			memcpy( node->dirp->dir[ index ]->name, name, strlen( name ) + 1);
			memcpy( fs_root[ vfs_i_count ].name, name, strlen( name ) + 1);

			fs_root[ vfs_i_count ].inode 	= vfs_i_count;
			fs_root[ vfs_i_count ].type 	= FS_FILE;
			fs_root[ vfs_i_count ].read 	= vfs_read;
			fs_root[ vfs_i_count ].pread 	= vfs_pread;
			fs_root[ vfs_i_count ].write 	= vfs_write;
			fs_root[ vfs_i_count ].pwrite 	= vfs_pwrite;

			node->dirp->dir_count++;
			vfs_i_count++;
			return 0;
		} else {
			return -1;
		}
	}
	return -1;
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

file_node_t *fs_find_node( file_node_t *node, char *name, unsigned int links ){ DEBUG_HERE
	if ( node->dirp && node->find_node ){ DEBUG_HERE
		return node->find_node( node, name, links );
	} else { DEBUG_HERE
		return 0;
	}
}
#endif
