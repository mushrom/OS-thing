#ifndef _kernel_fs_h
#define _kernel_fs_h
#define MAX_NAME_LEN	256
#define MAX_DIRS 	256
#define MAX_INODES	64
#include <mem/alloc.h>
#include <lib/stdio.h>
#include <lib/string.h>
#include <lib/kmacros.h>

typedef enum {
	FS_FILE,
	FS_DIR,
	FS_CHAR_D,
	FS_BLOCK_D,
	FS_PIPE,
	FS_SYMLINK,
	FS_MOUNT,
} file_type_t;

enum {
	O_RDONLY	= 1,
	O_WRONLY	= 2,
	O_RDWR		= 4,
	O_EXEC		= 8,
	O_NONBLOCK	= 16,
	O_APPEND	= 32,
	O_CREAT		= 64,
	O_TRUNC		= 128
};
struct file_node;

typedef int (*open_func)( struct file_node *, char *, int);
typedef int (*write_func)( struct file_node *, void *, unsigned long );
typedef int (*read_func)( struct file_node *,  void *, unsigned long );
typedef int (*pwrite_func)( struct file_node *, void *, unsigned long, unsigned long );
typedef int (*pread_func)( struct file_node *, void *, unsigned long, unsigned long );
typedef int (*ioctl_func)( struct file_node *, unsigned long, ... );
typedef struct dirp *(*opendir_func)( struct file_node * );
typedef int (*closedir_func)( struct file_node * );
typedef int (*mkdir_func)( struct file_node *, char *, int );
typedef int (*close_func)( struct file_node * );
typedef struct file_node *(*find_node_func)( struct file_node *, char *name );

typedef struct file_node {
	file_type_t	type;

	unsigned char	name[ MAX_NAME_LEN ];
	unsigned long 	mask;
	unsigned long	uid;
	unsigned long 	gid;
	unsigned long	time;
	unsigned long	inode;
	unsigned long	size;
	unsigned long	links;
	unsigned long 	flags;
	unsigned long	maj_num;
	unsigned long	min_num;
	struct dirp 	*dirp;

	read_func  	read;
	pread_func  	pread;
	write_func	write;
	pwrite_func 	pwrite;
	ioctl_func  	ioctl;

	opendir_func	opendir;
	closedir_func	closedir;
	mkdir_func	mkdir;
	find_node_func	find_node;

	open_func	open;
	close_func	close;

	struct file_node *mount;
} file_node_t;

typedef struct file_header {
	void *data;
} file_header_t;

struct dirent {
	unsigned char 	name[ MAX_NAME_LEN ];
	unsigned long	inode;
};

struct dirp {
	struct dirent *dir[ MAX_DIRS ];
	unsigned long dir_count, dir_ptr;
};

void init_vfs( void );

/* Some generic functions for acting on fs nodes */
int  fs_write( file_node_t *, void *, unsigned long );
int   fs_read( file_node_t *, void *, unsigned long );
int fs_pwrite( file_node_t *, void *, unsigned long, unsigned long );
int  fs_pread( file_node_t *, void *, unsigned long, unsigned long );
int  fs_ioctl( file_node_t *, unsigned long, ... );
int   fs_open( file_node_t *, char *, int );
int  fs_close( file_node_t * );
file_node_t *fs_find_node( file_node_t *, char * );

struct dirp *fs_opendir( file_node_t * );
int fs_closedir( file_node_t * );
int fs_mkdir( file_node_t *, char *name, unsigned long );

struct dirent *fs_readdir( struct dirp *dir ); 


#endif