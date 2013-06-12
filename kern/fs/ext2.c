#ifndef _kernel_ext2fs_c
#define _kernle_ext2fs_c
#include <ext2.h>

void ext2fs_dump_info( char *path ){
	int fd = open( path, O_RDONLY );

	if ( fd < 0 ){ 
		printf( "[ext2] Could not open file\n" );
		return;
	}

	// freeme
	ext2_sprblk_t *sprblk = (ext2_sprblk_t *)kmalloc( 1024, 0, 0 );
	memset( sprblk, 0, 1024 );
	ext2_ex_sprblk_t *esprblk = (ext2_ex_sprblk_t *)((unsigned long)sprblk + 84);

	lseek( fd, 1024, 0 );
	read( fd, sprblk, 1024 );

	printf( "ext version: %d.%d\n", sprblk->maj_version, sprblk->min_version );
	printf( "n_inodes: %d, n_blocks: %d, reserved blocks: %d\n",
		sprblk->n_inodes, sprblk->n_blocks, sprblk->su_reserved );

	printf( "free_blocks: %d, free_inodes: %d, superblock: %d\n",
		sprblk->free_blocks, sprblk->free_inodes, sprblk->sprblk_block );

	printf( "block size: %d, fragment size: %d, blocks per group: %d\n", 
		1024 << sprblk->block_size, 1024 << sprblk->frag_size, sprblk->n_block_block_grp );

	printf( "First free inode: %d, sizeof inode: %d\n", esprblk->first_free_inode,
		esprblk->inode_size );

	printf( "last path: \"%s\"\n", esprblk->last_path );

	unsigned int block_groups = sprblk->n_blocks / sprblk->n_block_block_grp;
	block_groups += ( sprblk->n_blocks % sprblk->n_block_block_grp != 0 )?1:0;
	printf( "%u block groups\n", block_groups );

	// freeme
	ext2_block_desc_t *blkdesc = 
		(ext2_block_desc_t *)kmalloc( sizeof( ext2_block_desc_t ) * block_groups, 0, 0 );

	lseek( fd, (1024 << sprblk->block_size) * 2, 0 );
	read( fd, blkdesc, sizeof( ext2_block_desc_t ) * block_groups );

	printf( "first block group descript, inode table: %d\n", blkdesc->inode_table_addr );

	int inode_addr = ( 1024 << sprblk->block_size ) * blkdesc->inode_table_addr + 
			(2 - 1) * esprblk->inode_size;
	printf( "Trying 0x%x...\n", inode_addr );

	// freeme
	ext2_inode_t *inode = (ext2_inode_t *)kmalloc( esprblk->inode_size, 0 ,0 );
	lseek( fd, inode_addr, 0 );
	read( fd, inode, esprblk->inode_size );

	printf( "Got inode, links: %d, 1st block: %d\n", inode->hard_links, inode->d_ptr[0] );

	char *data = (char *)kmalloc(( 1024 << sprblk->block_size ), 0, 0 );
	lseek( fd, ( 1024 << sprblk->block_size ) * inode->d_ptr[0], 0 );
	read( fd, data, ( 1024 << sprblk->block_size ));

	printf( "root listing:\n" );
	ext2_dirent_t *dirent = (ext2_dirent_t *)data;
	while( *(char *)dirent ){
		printf( "name: \"%s\", inode: %d\n", &dirent->name, dirent->inode );
		dirent = (ext2_dirent_t *)(((unsigned long)dirent) + dirent->size );
	}

	kfree( inode );
	kfree( blkdesc );
	kfree( sprblk );
	return;
}

#endif
