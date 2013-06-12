#ifndef _kernel_initrd_c
#define _kernel_initrd_c
#include <initrd.h>

file_node_t  **initrd_nodes;
file_node_t  *initrd_root;
initrd_file_header_t *initrd_files;
char *initrd_data;

unsigned long 	initrd_i_count = 0,
		data_offset = 0;

int initrd_read( file_node_t *node, void *buf, unsigned long size );
int initrd_pread( file_node_t *node, void *buf, unsigned long size, unsigned long offset );
int initrd_open( file_node_t *node, char *path, int flags );
int initrd_close( file_node_t *node );
file_node_t *initrd_find_node( file_node_t *node, char *name, unsigned int links ); 

file_node_t *initrd_create( initrd_header_t *header ){
	if (!( header->magic == INITRD_MAGIC && header->nfiles )){
		printf( "    Could not load initrd, magic: 0x%x, nfiles: %d\n", header->magic, header->nfiles );
		return 0;
	}
	unsigned long i = 0, j = 0;

	//printf( "    magic: 0x%x, nfiles: %d\n", header->magic, header->nfiles );

	//initrd_files = (initrd_file_header_t *)((unsigned long)header + sizeof( initrd_header_t ));
	/*
	for ( i = 0; i < header->nfiles + 1; i++ )
		initrd_nodes[i] = (void *)kmalloc( sizeof( file_node_t ), 0, 0 );

	initrd_root = initrd_nodes[0];
	*/
	
	//memcpy( initrd_root->name, "initrd", 7 );
	//initrd_root->dirp = (void *)kmalloc( sizeof( struct dirp ), 0, 0 );
	//memset( initrd_root->dirp, 0, sizeof( struct dirp ));

	//initrd_root->inode = initrd_i_count++;
	/*
	initrd_root->opendir 	= vfs_opendir;
	initrd_root->closedir	= vfs_closedir;
	*/
	/*
	initrd_root->open	= initrd_open;
	initrd_root->close	= initrd_close;
	initrd_root->find_node	= initrd_find_node;
	initrd_root->type	= FS_DIR;
	initrd_root->mask	= 0755;
	*/

	//for ( i = 0; i < header->nfiles; i++, initrd_root->dirp->dir_ptr++ ){
	/*
	for ( i = 0; i < header->nfiles; i++ ){
		//initrd_root->dirp->dir[i] = (void *)kmalloc( sizeof( struct dirent ), 0, 0 );
		//memset( initrd_root->dirp->dir[i], 0, sizeof( struct dirent ));
		j = initrd_i_count;
		memset( initrd_nodes[j], 0, sizeof( file_node_t ));
		memcpy( initrd_nodes[j]->name, files[i].name, strlen( files[i].name ) + 1);
		//memcpy( initrd_root->dirp->dir[i]->name, files[i].name, strlen( files[i].name ) + 1);
		
		initrd_nodes[j]->inode = initrd_i_count;

		initrd_nodes[j]->mask		= 0555;
		initrd_nodes[j]->size		= files[i].length;
		initrd_nodes[j]->type		= files[i].type;
		initrd_nodes[j]->read		= initrd_read;
		initrd_nodes[j]->open		= initrd_open;
		initrd_nodes[j]->close		= initrd_close;
		initrd_nodes[j]->pread		= initrd_pread;
		initrd_nodes[j]->find_node	= initrd_find_node;

		printf( "    file %s, size: %d:%d\n", initrd_nodes[j]->name, files[i].length, files[i].offset );
		//printf( ":%d:%d\n", initrd_nodes[j]->read, 0 );
	}
	*/

	//initrd_root->dirp->dir_ptr = 0;
	/*
	data_offset = ( sizeof( initrd_file_header_t ) * header->nfiles ) + sizeof( initrd_header_t );
	data = (char *)header;
	*/

	return initrd_root;
	//extern file_node_t *fs_root;
	//file_node_t *temp = fs_find_node( fs_root, "init", 1 );
	//file_node_t *temp = fs_find_path( "/sbin", 1 );
	//file_node_t *temp = fs_find_path( "/init", 1 );
	/*
	if ( temp )
		temp->mount = initrd_root;
	*/

	/*
	if ( temp )
		fs_mount( initrd_root, temp, 0, 0 );
	*/

	/*
	printf( "\x17%x:%x:%d\n", data, header, i );
	for ( i = 0; i < 28; i++ )
		printf( "%d:0x%x ", i, data[i + data_offset] );
	*/

}

file_node_t *initrd_find_node( file_node_t *node, char *name, unsigned int links ){ 
	int i = 0, has_subdir = 0;
	file_node_t *ret = 0;
	char *sub_dir = 0;

	if ( node->type != FS_DIR )
		return 0;

	for ( i = 0; i < strlen( name ); i++ ){ 
		if ( name[i] == '/' ){ 
			has_subdir = 1;
			name[i] = 0;
			sub_dir = name + i + 1;
			break;
		}
	}

	struct dirent *dir = (struct dirent *)&initrd_data[ data_offset + initrd_files[node->inode-1].offset + i ];
	int dir_size = node->size / sizeof( struct dirent );
	for ( i = 0; i < dir_size; i++ ){ 
		if ( strcmp( dir[i].name, name ) == 0 ){ 
			ret = initrd_nodes[ dir[i].inode ];
			//printf( "0x%x:%s:%d:0x%x\n", ret->read, ret->name, ret->inode, ret->mask );
			if ( has_subdir ){ 
				if ( node->find_node ){ 
					return node->find_node( ret, sub_dir, 1 );
				} else { 
					return 0;
				}
			} else { 
				if ( ret->mount ){
					return ret->mount;
				} else {
					//printf( "%x\n", ret );
					return ret;
				}
			}
		}
	}
	/*
	for ( i = 0; i < node->dirp->dir_count; i++ ){ 
		if ( strcmp((char *)node->dirp->dir[i]->name, name ) == 0 ){ 
			ret = initrd_nodes[ node->dirp->dir[i]->inode ];
			//printf( "0x%x:%s:%d:0x%x\n", ret->read, ret->name, ret->inode, ret->mask );
			if ( has_subdir ){ 
				if ( node->find_node ){ 
					return node->find_node( ret, sub_dir, 1 );
				} else { 
					return 0;
				}
			} else { 
				if ( ret->mount ){
					return ret->mount;
				} else {
					//printf( "%x\n", ret );
					return ret;
				}
			}
		}
	}
	*/
	return 0;
}

int initrd_open( file_node_t *node, char *path, int flags ){
	return node->inode;
}

int initrd_close( file_node_t *node ){
	return 0;
}

int initrd_read( file_node_t *node, void *buf, unsigned long size ){
	char *output 	= buf;
	unsigned int i, inode = node->inode;
	
	for ( i = 0; i < size && i < node->size; i++ ){
		output[i] = initrd_data[ data_offset + initrd_files[inode].offset + i ];
	}
	return i;
}

int initrd_pread( file_node_t *node, void *buf, unsigned long size, unsigned long offset ){
	char *output 	= buf;
	unsigned int i, inode = node->inode;

	for ( i = offset; i < size + offset && i < node->size; i++ ){
		output[i-offset] = initrd_data[ data_offset + initrd_files[inode].offset + i ];
	}
	return i - offset;
}

#endif
