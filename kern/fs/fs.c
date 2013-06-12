#ifndef _kernel_fs_c
#define _kernel_fs_c
#include <fs.h>

file_node_t 	*fs_root;
extern task_t *current_task;

void set_fs_root( file_node_t *new_root ){
	fs_root = new_root;
}

file_node_t *fs_find_path( char *input_path, unsigned int links ){
	char *path, *p; 
	file_node_t *fp, *ret;
	if ( strlen( input_path ) == 0 )
		input_path = ".";

	p = path = (void *)kmalloc( strlen( input_path + 1 ), 0, 0 );
	memcpy( path, input_path, strlen( input_path ) + 1 );

	if ( path[0] == '/' ){
		if ( current_task )
			fp = current_task->root;
		else
			fp = fs_root;

		while ( path[0] == '/' )
			path++;

		if ( strlen( path ) == 0 )
			return fp;
	} else if ( path[0] == '.' ){
		fp = current_task->cwd;
		if ( strlen( path ) == 1 )
			return fp;
		path++;
		while ( path[0] == '/' )
			path++;
	} else {
		if ( current_task ){
		//	path = current_task->cwd->name;
			fp = current_task->cwd;
		} else {
		//	path = fs_root->name;
			fp = fs_root;
		}
	}

	ret = fs_find_node( fp, path, links );
	kfree( p );
	return ret;
}

int isgoodfd( task_t *task, int fd ){
	if ( !task->file_count || fd >= task->file_count )
		return 0;

	if ( !task->files[fd] )
		return 0;
		//exit_thread();

	return 1;
}

int check_perms( task_t *task, file_node_t *node, int flags ){
	int perms = 0000;
	int good_op = 1;

	if ( task->uid == node->uid ){
		perms = node->mask >> P_USHIFT & 07;
	} else if ( task->gid == node->gid ){
		perms = node->mask >> P_GSHIFT & 07;
	} else {
		perms = node->mask >> P_ESHIFT & 07;
	}

	if ( !flags )
		good_op = 0;

	if ( flags & O_RDONLY && !( perms & P_READ ))
		good_op = 0;

	if ( flags & O_WRONLY && !( perms & P_WRITE ))
		good_op = 0;

	if ( flags & O_RDWR && !( perms & P_READ && perms & P_WRITE ))
		good_op = 0;

	if ( flags & O_EXEC && !( perms & P_EXEC ))
		good_op = 0;
	
	return good_op;
}
	

int open( char *path, int flags ){
	file_node_t *fp = fs_find_path( path, 1 );
	unsigned long i = 0;
	char *p, *temp;
	int ret = 0, found;
	
	if ( !fp ){
		found = 0;
		if ( flags & O_CREAT ){
			temp = path;
			p = temp = (void *)kmalloc( strlen( path + 1 ), 0, 0 );
			memcpy( temp, path, strlen( path ) + 1 );
			for ( ; *p; p++ );
			for ( ; p >= temp; p-- ){
				if ( *p == '/' || p == temp ){
					*p = 0;
					break;
				}
			}
			//printf( "Researching for \"%s\"\n", temp );
			fp = fs_find_path( temp, 1 );
			kfree( temp );
			if ( !fp )
				return -1;
		} else {
			
			return -1;
		}
	} else {
		found = 1;
	}

	if ( current_task->file_count >= MAX_FILES )
		return -1;
	
	for ( i = 0; i < MAX_FILES; i++ ){
		if ( current_task->files[i] == 0 )
			break;
	}

	if ( i == MAX_FILES )
		return -1;

	for ( temp = p = path; *p; p++ ){
		if ( *p == '/' )
			temp = p + 1;
	}

	if ( !check_perms( current_task, fp, flags ))
		return -1;

	//printf( "opening \"%s\"\n", temp );
	ret = fs_open( fp, temp, flags );

	if ( ret < 0 )
		return ret;

	if ( !found )
		fp = fs_find_path( path, 1 );

	if ( !fp )
		return -1;
	
	current_task->files[i] = (void *)kmalloc( sizeof( file_descript_t ), 0, 0 );
	current_task->files[i]->file = fp;
	current_task->files[i]->r_offset = 0;
	current_task->files[i]->w_offset = 0;
	current_task->files[i]->d_offset = 0;
	current_task->file_count++;	
	
	return i;
}

/* TODO:
 * 	get rid of these
 */
/*
struct dirp *vfs_opendir( file_node_t *node ){ DEBUG_HERE
	node->dirp->dir_ptr = 0;
	return node->dirp;
}

int vfs_closedir( file_node_t *node ){ DEBUG_HERE
	return 0;
}
*/

int close( int fd ){
	if ( !isgoodfd( current_task, fd ))
		return -1;

	kfree( current_task->files[fd] );

	int ret;

	ret = fs_close( current_task->files[fd]->file );
	current_task->files[fd] = 0;
	current_task->file_count--;
	return ret;
}

int read( int fd, void *buf, unsigned long size ){
	if ( !isgoodfd( current_task, fd ))
		return -1;
	
	unsigned long i = 0, offset = current_task->files[fd]->r_offset;
	i = fs_pread( current_task->files[fd]->file, buf, size, offset );
	current_task->files[fd]->r_offset += i;

	return i;
}

int write( int fd, void *buf, unsigned long size ){
	if ( !isgoodfd( current_task, fd ))
		return -1;

	unsigned long i = 0, offset = current_task->files[fd]->w_offset;
	i = fs_pwrite( current_task->files[fd]->file, buf, size, offset );
	if ( i > 0 )
		current_task->files[fd]->w_offset += i;

	return i;
}

/*
struct dirp *fdopendir_c( int fd, struct dirp *buf ){
	if ( !isgoodfd( current_task, fd ))
		return -1;

	if ( !current_task->files[fd]->file->dirp )
		return 0;

	current_task->files[fd]->d_offset = 0;
	
	memcpy( buf, current_task->files[fd]->file->dirp, sizeof( struct dirp ));
	return buf;
}

struct dirent *readdir_c( int fd, struct dirp *dir, struct dirent *buf ){
	if ( !isgoodfd( current_task, fd ))
		return -1;

	if ( !dir || current_task->files[fd]->d_offset >= dir->dir_count )
		return 0;

	memcpy( buf, dir->dir[ current_task->files[fd]->d_offset++ ], sizeof( struct dirent ));
	return buf;
}
*/

int chdir( char *path ){
	file_node_t *fp = fs_find_path( path, 1 );
	if ( !fp )
		return -1;

	current_task->cwd = fp;

	return 0;
}

int chroot( char *path ){
	file_node_t *fp = fs_find_path( path, 1 );
	if ( !fp )
		return -1;

	current_task->root = fp;

	return 0;
}

int lstat( char *path, struct vfs_stat *buf ){
	file_node_t *fp = fs_find_path( path, 1 );
	if ( !fp )
		return -1;

	return fs_stat( fp, buf );
}

int lseek( int fd, long offset, int whence ){
	int size;
	if ( !isgoodfd( current_task, fd ))
		return -1;

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

/*
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
*/

int fs_mkdir ( file_node_t *node, char *name, unsigned long mode ){ DEBUG_HERE
	if ( node->type == FS_DIR && node->mkdir ){ DEBUG_HERE
		return node->mkdir( node, name, mode );
	} else { DEBUG_HERE
		return -1;
	}
}

int fs_mknod ( file_node_t *node, char *name, int mode, int dev ){ DEBUG_HERE
	if ( node->type == FS_DIR && node->mknod ){ DEBUG_HERE
		return node->mknod( node, name, mode, dev );
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
	if ( node->type == FS_DIR && node->find_node ){ DEBUG_HERE
		return node->find_node( node, name, links );
	} else { DEBUG_HERE
		return 0;
	}
}
#endif
