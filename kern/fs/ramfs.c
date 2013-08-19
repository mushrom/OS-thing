#ifndef _kernel_ramfs_c
#define _kernel_ramfs_c
#include <ramfs.h>

int ramfs_open ( file_node_t *, char *, int );
int ramfs_close( file_node_t * );
int ramfs_read( file_node_t *node, void *buf, unsigned long size );
int ramfs_pread( file_node_t *node, void *buf, unsigned long size, unsigned long offset );
int ramfs_write( file_node_t *node, void *buf, unsigned long size );
int ramfs_pwrite( file_node_t *node, void *buf, unsigned long size, unsigned long offset );

int ramfs_mknod( file_node_t *node, char *path, int mode, int dev );
int ramfs_mkdir_entry( file_node_t *node, char *name, int inode );
int ramfs_mkdir( file_node_t *, char *, int );

int ramfs_unlink( file_node_t *node );
int ramfs_get_info( file_node_t *node, file_info_t *buf );
int ramfs_callback( file_node_t *node, int op, int var, int value );
int ramfs_readdir( file_node_t *node, struct dirent *dirp, int entry );

file_funcs_t ramfs_file_funcs = {
	.read 	= ramfs_read,
	.pread 	= ramfs_pread,
	.write	= ramfs_write,
	.pwrite	= ramfs_pwrite,
	.ioctl	=  0, // FIXME

	.mkdir	= ramfs_mkdir,
	.mknod	= ramfs_mknod,
	.link	= 0, // FIXME
	.unlink	= ramfs_unlink,
	.readdir = ramfs_readdir,

	.open	= ramfs_open,
	.close 	= ramfs_close,

	.get_info = ramfs_get_info,
	.callback = ramfs_callback,
	
};

file_system_t *init_ramfs( void ){ DEBUG_HERE

	ramfs_file_header_t 	*ramfs_root;
	ramfs_header_t *head	= knew( ramfs_header_t );
	file_system_t *ret	= knew( file_system_t );

	head->ramfs_files 	= knew( ramfs_file_header_t );

	ramfs_root = head->ramfs_files;
	head->ramfs_i_count = 1;

	memcpy( ramfs_root->name, "root", 5 );
	ramfs_root->type 	= FS_DIR;
	ramfs_root->mask 	= 0777;
	ramfs_root->inode	= head->ramfs_i_count++;
	ramfs_root->size    	= 0;
	ramfs_root->links   	= 1;

	ret->name = "ramfs";
	ret->i_root = ramfs_root->inode;
	ret->fs_data = head;
	ret->ops = &ramfs_file_funcs;

	return ret;
}

ramfs_file_header_t *ramfs_get_file_h( ramfs_header_t *head, int inode ){
	ramfs_file_header_t 	*temp = head->ramfs_files,
				*move = 0;

	for ( ; temp; temp = temp->next ){
		if ( temp->inode == inode ){
			move = temp;
			break;
		}
	}

	return move;
}

int ramfs_get_info( file_node_t *node, file_info_t *buf ){
	ramfs_file_header_t *filehead = ramfs_get_file_h( node->fs->fs_data, node->inode );

	if ( !filehead )
		return -ENOENT;

	buf->type	= filehead->type;

	buf->name 	= filehead->name;
	buf->mask 	= filehead->mask;
	buf->uid	= filehead->uid;
	buf->gid	= filehead->gid;
	buf->time	= filehead->gid;
	buf->inode	= node->inode;
	buf->size	= filehead->size;
	buf->links	= filehead->links;
	buf->flags	= filehead->flags;

	buf->fs		= node->fs;
	//buf->mount	= filehead->mount;
	buf->mount_id	= filehead->mount_id;

	return 0;
}

int ramfs_read( file_node_t *node, void *buf, unsigned long size ){
	return 0;
}

int ramfs_pread( file_node_t *node, void *buf, unsigned long size, unsigned long offset ){ DEBUG_HERE
	char *output	= buf;
	ramfs_file_header_t *filehead = ramfs_get_file_h( node->fs->fs_data, node->inode );
	char *file_data = filehead->data;
	int i, j;

	for ( i = offset, j = 0; j < size && i < filehead->size; j++, i++ ){
		output[j] = file_data[i];
	}

	return i - offset;
}

int ramfs_write( file_node_t *node, void *buf, unsigned long size ){
	return 0;
}

int ramfs_pwrite( file_node_t *node, void *buf, unsigned long size, unsigned long offset ){ DEBUG_HERE
	char *input	= buf;
	ramfs_header_t *head = node->fs->fs_data;
	ramfs_file_header_t *filehead = ramfs_get_file_h( node->fs->fs_data, node->inode );

	char *file_data = filehead->data;
	int  file_len  = filehead->size, i;
	//printf( "Writing to inode %d, length %d...\n", node->inode, node->size );

	if ( size > file_len ){
		//kfree( file_data ); // Enable this after debugging
		file_data = (void *)kmalloc( size + offset + 1, 0, 0 );
		//memcpy( new_data, file_data, node->size );
		//ramfs_files[ node->inode ].data = file_data;
		//head->ramfs_files[ node->inode ].data = file_data;
		filehead->data = file_data;
	}
	for ( i = offset; i < size + offset + 1; i++ ){
		file_data[i] = input[i - offset];
	}
	filehead->size = size;
	//printf( "Wrote to inode %d, length %d...\n", node->inode, node->size );
	return i;
}

int ramfs_mkdir_entry( file_node_t *node, char *name, int inode ){
	ramfs_file_header_t *filehead = ramfs_get_file_h( node->fs->fs_data, node->inode );

	if ( filehead->type != FS_DIR )
		return -1;

	//struct dirent *dir;
	char *file_data = filehead->data;
	ramfs_dirent_t *dir, *temp = 0;

	int nlen = strlen( name ) + 1;
	unsigned int dsize = sizeof( struct dirent ) + nlen;

	ramfs_header_t *head = node->fs->fs_data;
	dir = knew( ramfs_dirent_t );

	printf( "doot" );

	dir->inode = inode;
	dir->size = dsize;
	dir->name_len = nlen;
	dir->name = knew( char[ strlen( name )]);
	memcpy( dir->name, name, nlen );

	printf( "deet" );
	
	if ( !filehead->data ){
		filehead->data = dir;
	} else {
		printf( "diit" );
		for ( temp = (ramfs_dirent_t *)file_data; temp->next; temp = temp->next );
		temp->next = dir;
	}

	filehead->size += dir->size;

	printf( "daat\n" );
	return 0;
}

int ramfs_mkdir( file_node_t *node, char *name, int mode ){ DEBUG_HERE
	ramfs_header_t *head = node->fs->fs_data;
	ramfs_file_header_t *filehead = ramfs_get_file_h( head, node->inode );
	ramfs_file_header_t *temp, *move;

	unsigned long ramfs_i_count = head->ramfs_i_count;

	if ( filehead->type != FS_DIR )
		return -1;

	ramfs_mkdir_entry( node, name, head->ramfs_i_count );
	temp = knew( ramfs_file_header_t );

	temp->mask	= mode;
	temp->type 	= FS_DIR;

	temp->inode 	= ramfs_i_count;
	temp->parent 	= node;
	temp->links	= 1;
	temp->data	= 0;

	memcpy( head->ramfs_files[ head->ramfs_i_count ].name, name, strlen( name + 1 ));

	for ( move = filehead; move->next; move = move->next );
	move->next = temp;

	head->ramfs_i_count++;
	return 0;
}

int ramfs_mknod( file_node_t *node, char *name, int mode, int dev ){ DEBUG_HERE
	ramfs_header_t *head = node->fs->fs_data;
	ramfs_file_header_t *filehead = ramfs_get_file_h( head, node->inode );
	ramfs_file_header_t *temp, *move;

	printf( "[ ] Here from ramfs_mknod... 0x%x\n", head );
	unsigned long ramfs_i_count = head->ramfs_i_count;

	if ( filehead->type != FS_DIR )
		return -1;

	ramfs_mkdir_entry( node, name, head->ramfs_i_count );
	temp = knew( ramfs_file_header_t );

	temp->mask	= mode;
	temp->inode 	= ramfs_i_count;

	temp->type 	= dev;
	temp->size 	= 0;
	temp->parent	= node;
	temp->links	= 1;

	memcpy( head->ramfs_files[ ramfs_i_count ].name, name, strlen( name ) + 1 );

	for ( move = filehead; move->next; move = move->next );
	move->next = temp;

	head->ramfs_i_count++;
	return 0;
}

int ramfs_readdir( file_node_t *node, struct dirent *dirp, int entry ){
	ramfs_header_t *head = node->fs->fs_data;
	ramfs_file_header_t *filehead = ramfs_get_file_h( head, node->inode );
	int ret = 1, d;

	if ( filehead->type != FS_DIR ){
		ret = -ENOTDIR;
		goto end;
	}

	ramfs_dirent_t *dirty = (ramfs_dirent_t *)filehead->data;

	for ( d = 0; d < entry && dirty; d++, dirty = dirty->next );

	if ( !dirty ){
		ret = 0;
		goto end;
	}

	dirp->inode = dirty->inode;
	dirp->size = dirty->size;
	dirp->name_len = dirty->name_len;
	memcpy( dirp->name, dirty->name, dirty->name_len );
	
end:
	return ret;

}

int ramfs_open( file_node_t *node, char *name, int mode ){ DEBUG_HERE
	ramfs_file_header_t *filehead = ramfs_get_file_h( node->fs->fs_data, node->inode );

	if ( filehead->type == FS_DIR && mode & O_CREAT ){
		return ramfs_mknod( node, name, 0777, FS_FILE );
	}

	return node->inode;
}

int ramfs_close( file_node_t *node ){
	return 0;
}

int ramfs_unlink( file_node_t *node ){
	ramfs_file_header_t *filehead = ramfs_get_file_h( node->fs->fs_data, node->inode );

	if ( filehead->links )
		filehead->links--;

	return 0;
}

int ramfs_callback( file_node_t *node, int op, int var, int value ){
	ramfs_file_header_t *filehead = ramfs_get_file_h( node->fs->fs_data, node->inode );
	int ret = 0;

	switch( op ){
		case FOP_SET:
	
			switch( var ){
				case FVAR_MOUNT_ID:
					filehead->mount_id = value;
					break;
				default:
					ret = -ENOENT;
			}
			break;

		default: 
			ret = -ENOTSUP;
			break;
	}
	
alldone:
	return ret;

}


#endif
