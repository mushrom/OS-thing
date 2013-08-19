#ifndef _kernel_ext2fs_c
#define _kernle_ext2fs_c
#include <ext2.h>

ext2_device_t *ext2_create_device( char *path );

ext2_block_desc_t *ext2_get_block_desc( ext2_device_t *dev, int group, ext2_block_desc_t *buf );
int ext2_read_block( ext2_device_t *dev, int block_addr, char *buf, unsigned int offset, unsigned int size );

//int ext2_get_inode_block_grp( ext2_device_t *dev, int inode );
int ext2_get_inode( ext2_device_t *dev, int inode, ext2_inode_t *buf );
int ext2_read_inode( ext2_device_t *dev, ext2_inode_t *inode, char *buf, unsigned int size );

file_node_t *ext2_create( char *path ){
	file_node_t *ret = knew( file_node_t );

	ret->fs = knew( file_system_t );

	ret->fs->fs_data = ext2_create_device( path );
	ret->inode = 2;
	//ret->parent = ret;

	//memcpy( ret->name, "root", 5 );

	return ret;
}

ext2_device_t *ext2_create_device( char *path ){
	ext2_device_t *ret = knew( ext2_device_t );

	ret->fd = open( path, O_RDONLY );
	if ( ret->fd < 0 ){ 
		printf( "[ext2] Could not open file\n" );
		return 0;
	}

	ret->sprblk = knew( char[1024] );
	ret->esprblk = (ext2_ex_sprblk_t *)((unsigned long)ret->sprblk + 84);

	lseek( ret->fd, 1024, 0 );
	read( ret->fd, ret->sprblk, 1024 );

	ret->block_size = ( 1024 << ret->sprblk->block_size );

	ret->block_groups = ret->sprblk->n_blocks / ret->sprblk->n_block_block_grp;
	ret->block_groups += ( ret->sprblk->n_blocks % ret->sprblk->n_block_block_grp != 0 )?1:0;

	ret->blkdesc = knew( ext2_block_desc_t[ ret->block_groups ]);

	lseek( ret->fd, ret->block_size * 2, 0 );
	read( ret->fd, ret->blkdesc, sizeof( ext2_block_desc_t ) * ret->block_groups );

	return ret;
}

int ext2_read_block( ext2_device_t *dev, int block_addr, char *buf, unsigned int offset, unsigned int size ){
	lseek( dev->fd, dev->block_size * block_addr + offset, 0 );
	return read( dev->fd, buf, dev->block_size - offset - ( dev->block_size - size ));
}

int ext2_get_inode( ext2_device_t *dev, int inode, ext2_inode_t *buf ){
	int inode_addr = dev->block_size * dev->blkdesc->inode_table_addr + 
			(inode - 1) * dev->esprblk->inode_size;

	lseek( dev->fd, inode_addr, 0 );
	read( dev->fd, buf, dev->esprblk->inode_size );

	return 0;
}

int ext2_read_inode( ext2_device_t *dev, ext2_inode_t *inode, char *buf, unsigned int size ){

	unsigned int i, ptr, readsize, total = 0;

	for ( i = ptr = 0; i < size && ptr < 12; i += dev->block_size, ptr++ ){
		if ( size - i >= dev->block_size )
			readsize = dev->block_size;
		else
			readsize = size % dev->block_size;

		total += ext2_read_block( dev, inode->d_ptr[ptr], buf + i, 0, dev->block_size );
	}

	return total;
}

/*
file_node_t *ext2_create_fnode( ext2_device_t *dev, int inode ){
	ext2_inode_t *inode_buf = knew( char[ dev->esprblk->inode_size ]);
	file_node_t *ret = knew( char[ dev->esprblk->inode_size ] );

	ext2_get_inode( dev, inode, inode_buf );
	ret->uid = inode->uid;
	
	return 0;

}
*/

/*
file_node_t *ext2fs_find_node( file_node_t *node, char *name, unsigned int links ){
	file_node_t *ret = 0;
	struct ext2_dirent *dir;
	char *buf = (char *)kmalloc( 4096, 0, 0 );
	int bread = 0,  i;
	char *subdir;

	for ( i = 0; i < strlen( name ); i++ ){
		if ( name[i] == '/' ){
			name[i] = 0;
			subdir = name + i + 1;
			break;
		}
	}
	
	bread = ext2_read_inode( node->fs->fs_data, node->inode, 4096 );
	if ( !bread || bread < 0 )
		return 0;

	dir = buf;
	
	return 0;
}
*/

void ext2fs_dump_info( char *path ){
	ext2_device_t *e2 = ext2_create_device( path );

	printf( "ext version: %d.%d\n", e2->sprblk->maj_version, e2->sprblk->min_version );
	printf( "n_inodes: %d, n_blocks: %d, reserved blocks: %d\n",
		e2->sprblk->n_inodes, e2->sprblk->n_blocks, e2->sprblk->su_reserved );

	printf( "free_blocks: %d, free_inodes: %d, superblock: %d\n",
		e2->sprblk->free_blocks, e2->sprblk->free_inodes, e2->sprblk->sprblk_block );

	printf( "block size: %d, fragment size: %d, blocks per group: %d\n", 
		1024 << e2->sprblk->block_size, 1024 << e2->sprblk->frag_size, 
		e2->sprblk->n_block_block_grp );

	printf( "First free inode: %d, sizeof inode: %d\n", e2->esprblk->first_free_inode,
		e2->esprblk->inode_size );

	printf( "last path: \"%s\"\n", e2->esprblk->last_path );

	printf( "first block group descript, inode table: %d\n", e2->blkdesc->inode_table_addr );

	ext2_inode_t *inode = knew( char[ e2->esprblk->inode_size ]);
	ext2_get_inode( e2, 2, inode );

	printf( "Got inode, links: %d, 1st block: %d\n", inode->hard_links, inode->d_ptr[0] );

	char *data = knew( char[ e2->block_size * 5 ] );
	ext2_read_inode( e2, inode, data, e2->block_size );

	printf( "root listing:\n" );
	ext2_dirent_t *dirent = (ext2_dirent_t *)data;
	while( *(char *)dirent ){
		printf( "name: \"%s\", inode: %d\n", &dirent->name, dirent->inode );
		dirent = (ext2_dirent_t *)(((unsigned long)dirent) + dirent->size );
	}

	//ext2_get_inode( e2, 2049, inode );
	//ext2_read_inode( e2, inode, data, e2->block_size * 3, 0 );
	//ext2_read_inode( e2, inode, data, 4279 );

	//printf( "size: %d\n", inode->low_size );
	//printf( "%s\n", data );

	//printf( "listing asdf:\n" );
	/*
	dirent = (ext2_dirent_t *)data;
	while( *(char *)dirent ){
		printf( "name: \"%s\", inode: %d\n", &dirent->name, dirent->inode );
		dirent = (ext2_dirent_t *)(((unsigned long)dirent) + dirent->size );
	}
	*/

	kfree( data );
	kfree( e2->sprblk );
	kfree( e2->blkdesc );
	kfree( e2 );
	return;
}

#endif
