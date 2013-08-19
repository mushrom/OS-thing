#ifndef _kernel_fs_c
#define _kernel_fs_c
#include <fs.h>

file_node_t 	*fs_root;
extern task_t *current_task;
llist_node_t *file_system_list = 0;
llist_node_t *mount_list = 0;
unsigned int mount_ids = 1;

void set_fs_root( file_system_t *new_root ){
	if ( !fs_root )
		fs_root = knew( file_node_t );

	fs_root->inode = new_root->i_root;
	fs_root->fs = new_root;
}

int fs_find_path( char *input_path, unsigned int links, file_node_t *buf ){
	char *path, *p; 
	int ret = 0;
	file_node_t *fp;
	if ( strlen( input_path ) == 0 )
		input_path = ".";

	p = path = (void *)kmalloc( strlen( input_path + 1 ), 0, 0 );
	memcpy( path, input_path, strlen( input_path ) + 1 );
	//printf( "[fs_find_path] path=\"%s\", ", path );

	if ( path[0] == '/' ){
		if ( current_task )
			fp = current_task->root;
		else
			fp = fs_root;

		while ( path[0] == '/' )
			path++;

		if ( strlen( path ) == 0 ){
			buf->inode = fp->inode;
			buf->fs	= fp->fs;
			printf( "got here, " );
			goto alldone;
		}
	} else {
		if ( current_task ){
			fp = current_task->cwd;
		} else {
			fp = fs_root;
		}
	}

	//ret = knew( file_node_t );
	//printf( "[fs_find_path] Searching for \"%s\"\n", path );
	ret = fs_find_node( fp, path, links, buf );

alldone:
	kfree( p );

	printf( "[fs_find_path] buf->fs=0x%x, ret=0x%x\n", buf->fs, ret );
	return ret;
}

int isgoodfd( task_t *task, int fd ){
	if ( !task->file_count || fd >= task->file_count ){
		return 0;
	}

	if ( !task->files[fd] ){
		return 0;
	}

	return 1;
}

int check_perms( task_t *task, file_node_t *node, int flags ){
	int perms = 0000;
	int good_op = 1;

	file_info_t *n_info = knew( file_info_t );
	node->fs->ops->get_info( node, n_info );

	if ( task->uid == n_info->uid ){
		perms = n_info->mask >> P_USHIFT & 07;
	} else if ( task->gid == n_info->gid ){
		perms = n_info->mask >> P_GSHIFT & 07;
	} else {
		perms = n_info->mask >> P_ESHIFT & 07;
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
	unsigned long i = 0;
	char *p, *temp;
	int found, err_val = 0;
	file_node_t *fp = knew( file_node_t );

	err_val = fs_find_path( path, flags, fp );
	
	if ( err_val < 0 ){
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
			err_val = fs_find_path( temp, flags, fp );
			kfree( temp );
			if ( err_val < 0 )
				goto finished;

		} else {
			err_val = -EPERM;
			goto finished;
		}
	} else {
		found = 1;
	}

	for ( temp = p = path; *p; p++ ){
		if ( *p == '/' )
			temp = p + 1;
	}

	if ( !check_perms( current_task, fp, flags )){
		err_val = -EACCES;
		goto finished;
	}

	//printf( "opening \"%s\"\n", temp );
	err_val = fp->fs->ops->open( fp, temp, flags );

	if ( err_val < 0 )
		goto finished;

	if ( !found ){
		err_val = fs_find_path( path, 1, fp );
		if ( err_val < 0 )
			goto finished;
	}

	if ( current_task->file_count >= MAX_FILES ){
		err_val = -EMFILE;
		goto finished;
	}
	
	for ( i = 0; i < MAX_FILES; i++ ){
		if ( current_task->files[i] == 0 )
			break;
	}

	if ( i == MAX_FILES ){
		err_val = -EMFILE;
		goto finished;
	}

	current_task->files[i] = knew( file_descript_t );
	current_task->files[i]->file = fp;
	current_task->files[i]->r_offset = 0;
	current_task->files[i]->w_offset = 0;
	current_task->files[i]->d_offset = 0;
	current_task->file_count++;	
	err_val = i;

finished:
	kfree( fp );
	return err_val;
	
}

int close( int fd ){
	if ( !isgoodfd( current_task, fd ))
		return -ENOENT;

	kfree( current_task->files[fd] );

	int ret;
	file_node_t *fp = current_task->files[fd]->file;

	ret = fp->fs->ops->close( fp );
	current_task->files[fd] = 0;
	current_task->file_count--;

	return ret;
}

int read( int fd, void *buf, unsigned long size ){
	if ( !isgoodfd( current_task, fd )){
		printf( "[read] got here\n" );
		return -1;
	}
	
	unsigned long 	i = 0, 
			offset = current_task->files[fd]->r_offset;

	file_node_t *fp = current_task->files[fd]->file;

	i = fp->fs->ops->pread( fp, buf, size, offset );
	current_task->files[fd]->r_offset += i;

	return i;
}

int write( int fd, void *buf, unsigned long size ){
	if ( !isgoodfd( current_task, fd ))
		return -1;

	unsigned long 	i = 0, 
			offset = current_task->files[fd]->w_offset;

	file_node_t *fp = current_task->files[fd]->file;
	i = fp->fs->ops->pwrite( fp, buf, size, offset );

	if ( i > 0 )
		current_task->files[fd]->w_offset += i;

	return i;
}

int chdir( char *path ){
	file_node_t fp;
	int ret = fs_find_path( path, 1, &fp );

	if ( ret < 0 )
		return ret;

	current_task->cwd->inode = fp.inode;
	current_task->cwd->fs = fp.fs;

	return 0;
}

int chroot( char *path ){
	file_node_t fp;
	int ret = fs_find_path( path, 1, &fp );

	if ( ret < 0 )
		return ret;

	current_task->root->inode = fp.inode;
	current_task->root->fs = fp.fs;

	return 0;
}

int lstat( char *path, struct vfs_stat *buf ){
	file_node_t fp;
	int ret = fs_find_path( path, 1, &fp );

	if ( ret < 0 )
		return ret;

	file_info_t *n_info = knew( file_info_t );
	fp.fs->ops->get_info( &fp, n_info );
	
	buf->type = n_info->type;
	buf->uid = n_info->uid;
	buf->gid = n_info->gid;
	buf->time = n_info->time;
	buf->size = n_info->size;
	buf->mask = n_info->mask;

	kfree( n_info );
	return 0;
}

int lseek( int fd, long offset, int whence ){
	int size;
	if ( !isgoodfd( current_task, fd ))
		return -ENOENT;

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
			// TODO: put in proper size
			size = 0;
			//size = current_task->files[fd]->file->size;
			if ( size + offset < 0 )
				return -1;
			current_task->files[fd]->r_offset = size + offset;
			current_task->files[fd]->w_offset = size + offset;
			break;
		default:
			return -EINVAL;
	}

	return current_task->files[fd]->r_offset;
}

int mkdir( char *path, int mode ){
	file_node_t dir;
	char *file = 0;
	int i;

	int ret = fs_find_path( path, 1, &dir );
	printf( "Bloop\n" );

	/* If the path exists, return */
	if ( ret == 0 )
		return -EEXIST;

	/* Find first directory above the directory to make */
	for ( i = strlen( path ); i > 0; i-- ){
		if ( path[i] == '/' ){
			path[i] = 0;
			file = path + i + 1;
			break;
		}
	}
	
	/* Found a directory in the path, try and find the node */
	if ( i ){
		ret = fs_find_path( path, 1, &dir );

	/* No directory/was root, search current directory or root */
	} else { 
		file = path;
		if ( file[0] == '/' ){
			file++;
			ret = fs_find_path( "/", 1, &dir );
		} else {
			ret = fs_find_path( ".", 1, &dir );
		}
	}

	printf( "[mkdir 1] ret = 0x%x\n", ret );
	
	if ( ret < 0 )
		return ret;

	printf( "[mkdir 2] ret = 0x%x\n", ret );
	ret = dir.fs->ops->mkdir( &dir, file, mode );
	return ret;
}

int mknod( char *path, int mode, int dev ){
	file_node_t fp;
	char *file = 0;
	int i;

	int ret = fs_find_path( path, 1, &fp );

	/* If the path exists, return */
	if ( ret == 0 ){
		printf( "[mknod] Node exists\n" );
		return -EEXIST;
	}

	/* Find first directory above the directory to make */
	for ( i = strlen( path ); i > 0; i-- ){
		if ( path[i] == '/' ){
			path[i] = 0;
			file = path + i + 1;
			break;
		}
	}
	
	/* Found a directory in the path, try and find the node */
	if ( i ){
		ret = fs_find_path( path, 1, &fp );

	/* No directory/was root, search current directory or root */
	} else { 
		file = path;
		if ( file[0] == '/' ){
			file++;
			ret = fs_find_path( "/", 1, &fp );
		} else {
			ret = fs_find_path( ".", 1, &fp );
		}
	}

	printf( "[mknod 1] ret = 0x%x\n", ret );
	
	/*
	if ( ret < 0 )
		return ret;
	*/

	printf( "[mknod 2] ret = 0x%x\n", ret );
	ret = fp.fs->ops->mknod( &fp, file, mode, dev );
	return ret;
}

int unlink( char *path ){
	file_node_t fp;
	int ret = fs_find_path( path, 0, &fp );

	if ( ret < 0 )
		return ret;

	ret = fp.fs->ops->unlink( &fp );
	//return fs_unlink( fp );
	//return fs_call( FOP_UNLINK, fp );
	return ret;
}

/*
int getdents( int fd, struct dirent *dirp, unsigned int count ){
	int ret = 0, i, offset;
	file_node_t *fp;

	if ( !isgoodfd( current_task, fd )){
		ret = -ENOENT;
		goto alldone;
	}

	fp = current_task->files[fd]->file;
	offset = current_task->files[fd]->r_offset;

	i = fp->fs->ops->getdents( fp, dirp, count, offset );
	if ( i > -1 ){
		current_task->files[fd]->r_offset += i;
	} else {
		ret = i;
		goto alldone;
	}

	ret = i;
	
alldone:
	return ret;

}
*/
int readdir( int fd, struct dirent *dirp ){
	int ret = 0, i, offset;
	file_node_t *fp;
	file_descript_t *fdesc;

	if ( !isgoodfd( current_task, fd )){
		ret = -ENOENT;
		goto alldone;
	}

	fdesc = current_task->files[fd];
	fp = fdesc->file;
	offset = fdesc->d_offset++;

	ret = fp->fs->ops->readdir( fp, dirp, offset );

alldone:
	return ret;
}

int mount( char *type, char *dir, int flags, void *data ){
	file_node_t type_fp, dir_fp;
	int ret = 0, id;

	if (( ret = fs_find_path( type, 1, &type_fp )) < 0 )
		goto alldone;

	if (( ret = fs_find_path( dir, 1, &dir_fp )) < 0 )
		goto alldone;

	ret = fs_mount( &type_fp, &dir_fp, flags, data );

alldone:
	return ret;

}

int unmount( char *dir, int flags ){
	return -1;
	/*
	file_node_t *dir_fp;

	dir_fp  = fs_find_path( dir, 0 );
	if ( !dir_fp )
		return -1;

	//return fs_unmount( dir_fp, flags );
	return fs_call( FOP_UNMOUNT, dir_fp, flags );
	*/
}

/*
int fs_call( file_op_t op, file_node_t *node, ... ){
	va_list args;
	va_start( args, node );

	int ret;

	ret = node->fs->callback( op, node, args );
	
	va_end( args );
	return ret;
}
*/

int register_fs( file_system_t *newfs, int flags ){
	if ( !file_system_list )
		file_system_list = knew( llist_node_t );

	l_add_node_end( file_system_list, newfs );

	return 0;
}

int register_mount_node( file_node_t *node ){
	llist_node_t *move;
	file_node_t *blarg = knew( file_node_t );
	if ( !mount_list )
		mount_list = knew( llist_node_t );

	move = l_add_node_end( mount_list, 0 );

	blarg->fs = node->fs;
	blarg->inode = node->inode;

	move->data = blarg;
	move->val = ++mount_ids;

	return mount_ids;
}

int get_mount_node( int id, file_node_t *nodebuf ){
	llist_node_t *move = mount_list;
	file_node_t *blarg;

	for ( ; move; move = move->next ){
		if ( move->val == id ){
			blarg = (file_node_t *)move->data;
			nodebuf->fs = blarg->fs;
			nodebuf->inode = blarg->inode;
			return 1;
		}
	}

	return -ENOENT;
}

int fs_mount( file_node_t *type, file_node_t *dir, int flags, void *data ){
	int ret = 0, id;

	id = register_mount_node( dir );
	if ( id < 1 ){
		ret = id;
		goto alldone;
	}

	ret = type->fs->ops->callback( type, FOP_SET, FVAR_MOUNT_ID, id );

alldone:
	return ret;
	
}	

int fs_find_node( file_node_t *node, char *path, int links, file_node_t *nodebuf ){

	file_info_t *n_info = knew( file_info_t );
	int err_val = 0;

	int 	i, d = 0, r = 0, j = 0,
		found = 0,
		in_dir = 0, 
		has_subdir = 0;

	char 	*name = path,
		*nametemp = name,
		*nextname = name;

	struct dirent *dir = (struct dirent *)kmalloc( sizeof( dir ), 0, 0 );
	file_node_t temp;

	temp.fs = node->fs;
	temp.inode = node->inode;

	temp.fs->ops->get_info( &temp, n_info );

	while ( !found ){

		if ( !name ){
			err_val = -ENOENT;
			goto alldone;
		}

		nextname = 0;
		nametemp = name;
		for ( has_subdir = i = 0; name[i]; i++ ){
			if ( name[i] == '/' ){
				has_subdir = 1;
				name[i] = 0;
				nextname = name + i + 1;
				break;
			}
		}
		name = nametemp;

		if ( strcmp( name, "." ) == 0 ){
			name = nextname;
			if ( !has_subdir ){
				nodebuf->inode = temp.inode;
				nodebuf->fs = temp.fs;
				found = 1;
				break;
			}

			continue;
		} else if ( strcmp( name, ".." ) == 0 ){ 
			err_val = -ENOTDIR;
			goto alldone;
		} 

		printf( "[fs_find_node] Got here, inode=%d, subdir=%d, size=%d, name=\"%s\"\n", 
			n_info->inode, has_subdir, n_info->size, name );

		if ( n_info->type == FS_DIR ){
			printf( "[fs_find_node] trying [ " );
			d = r = in_dir = 0;

			while ( !in_dir ){
				r = temp.fs->ops->readdir( &temp, dir, d );

				if ( !r ) {
					printf( "read of 0 " );
					break;
				}

				d += r;

				if ( strcmp( dir->name, name ) == 0 ){
					temp.inode = dir->inode;
					in_dir = 1;
				}
			}

			printf( "]\n" );

			if ( !in_dir ){
				printf( "[fs_find_node] not in dir\n" );
				err_val = -ENOENT;
				goto alldone;

			} else {
				temp.fs->ops->get_info( &temp, n_info );

				if ( n_info->mount_id ){
					printf( "[fs_find_node] Followed mount_id %d\n", n_info->mount_id );
					get_mount_node( n_info->mount_id, &temp );
				}

				if ( !has_subdir ){
					nodebuf->inode = temp.inode;
					nodebuf->fs = temp.fs;
					found = 1;
				}
			}
				
		} else {
			err_val = -ENOTDIR;
			goto alldone;
		}

		name = nextname;
	}

alldone:
	kfree( n_info );
	kfree( dir );
	return err_val;

}

















#endif
