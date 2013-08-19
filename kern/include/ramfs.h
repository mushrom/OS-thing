#ifndef _kernel_ramfs_h
#define _kernel_ramfs_h
#include <fs.h>
#include <kmacros.h>

#define MAX_DIRS 	256
#define MAX_INODES	64

file_system_t *init_ramfs( void );
//int ramfs_callback( file_op_t op, file_node_t *node, va_list args );

typedef struct ramfs_dirent {
	unsigned long inode;
	unsigned long size;
	unsigned long name_len;
	char *name;

	struct ramfs_dirent *next;
} ramfs_dirent_t; 

typedef struct ramfs_file_header {
	file_type_t	type;

	char		name[ MAX_NAME_LEN ];
	unsigned long 	mask;
	unsigned long	uid;
	unsigned long 	gid;
	unsigned long	time;
	unsigned long	inode;
	unsigned long	size;
	unsigned long	links;
	unsigned long 	flags;

	unsigned long 	mount_id;
	struct file_node *parent;
	struct ramfs_file_header *next;

	void *data;
} ramfs_file_header_t;

typedef struct ramfs_header {
	struct ramfs_file_header *ramfs_files;
	unsigned long ramfs_i_count;
} ramfs_header_t;

#endif
