#ifndef _kernel_fs_h
#define _kernel_fs_h

#define MAX_NAME_LEN	256
#define MAX_DIRS 	256
#define MAX_INODES	64

#include <alloc.h>
#include <stdio.h>
#include <string.h>
#include <kmacros.h>
#include <task.h>

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

enum {
	P_READ = 4,
	P_WRITE = 2,
	P_EXEC = 1,

	P_USHIFT = 6,
	P_GSHIFT = 3,
	P_ESHIFT = 0,
};

struct file_node;
struct task;

typedef int (*open_func)( struct file_node *, char *, int);
typedef int (*write_func)( struct file_node *, void *, unsigned long );
typedef int (*read_func)( struct file_node *,  void *, unsigned long );
typedef int (*pwrite_func)( struct file_node *, void *, unsigned long, unsigned long );
typedef int (*pread_func)( struct file_node *, void *, unsigned long, unsigned long );
typedef int (*ioctl_func)( struct file_node *, unsigned long, ... );
typedef struct dirp *(*opendir_func)( struct file_node * );
typedef int (*closedir_func)( struct file_node * );
typedef int (*mkdir_func)( struct file_node *, char *, int );
typedef int (*mknod_func)( struct file_node *, char *, int, int );
typedef int (*close_func)( struct file_node * );
typedef struct file_node *(*find_node_func)( struct file_node *, char *name, unsigned int links );

/*! \brief The internal file node structure, every file has one. */
typedef struct file_node {
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
	unsigned long	dev_id;

	read_func  	read;
	pread_func  	pread;
	write_func	write;
	pwrite_func 	pwrite;
	ioctl_func  	ioctl;

	mkdir_func	mkdir;
	mknod_func	mknod;
	find_node_func	find_node;

	open_func	open;
	close_func	close;

	struct file_node *mount;
	struct file_node *parent;
} file_node_t;

typedef struct file_descript {
	file_node_t *file;
	unsigned long r_offset; /* read offset */
	unsigned long w_offset; /* write offset */
	unsigned long d_offset; /* directory offset */
} file_descript_t;

typedef struct vfs_file_header {
	void *data;
} vfs_file_header_t;

struct dirent {
	char 	name[ MAX_NAME_LEN ];
	unsigned long	inode;
};

/*
struct dirp {
	unsigned long dir_count, dir_ptr;
	struct dirent dir;
};
*/

struct vfs_stat {
	file_type_t 	type;

	unsigned long	uid;
	unsigned long	gid;
	unsigned long 	time;
	unsigned long 	size;
	unsigned long	mask;
};


/* Some generic functions for acting on fs nodes */
int  fs_write( file_node_t *, void *, unsigned long );
int   fs_read( file_node_t *, void *, unsigned long );
int fs_pwrite( file_node_t *, void *, unsigned long, unsigned long );
int  fs_pread( file_node_t *, void *, unsigned long, unsigned long );
int  fs_ioctl( file_node_t *, unsigned long, ... );
int   fs_open( file_node_t *, char *, int );
int  fs_close( file_node_t * );
int   fs_stat( file_node_t *, struct vfs_stat * );
int fs_mount( file_node_t *type, file_node_t *dir, int flags, void *data );
int fs_unmount( file_node_t *dir, int flags );
file_node_t *fs_find_node( file_node_t *, char *, unsigned int );

/*
struct dirp *fs_opendir( file_node_t * );
int fs_closedir( file_node_t * );
*/
int fs_mkdir( file_node_t *, char *name, unsigned long );
int fs_mknod( file_node_t *, char *name, int mode, int dev );

struct dirent *fs_readdir_c( struct dirp *dir, struct dirent *buf ); 

file_node_t *fs_find_path( char *path, unsigned int links );
int open( char *path, int flags );
int close( int fd );
int read( int fd, void *buf, unsigned long size );
int write( int fd, void *buf, unsigned long size );
struct dirp *fdopendir_c( int fd, struct dirp *dir );
struct dirent *readdir_c( int fd, struct dirp *dir, struct dirent *buf );
int mkdir( char *path, int mode );
int mknod( char *path, int mode, int dev );
int chdir( char *path );
int chroot( char *path );
int lseek( int fd, long offset, int whence );
int lstat( char *path, struct vfs_stat *buf );
int mount( char *type, char *dir, int flags, void *data );
int unmount( char *dir, int flags );

struct dirp *vfs_opendir( file_node_t * );
int vfs_closedir( file_node_t * );

int isgoodfd( struct task *task, int fd );
int check_perms( struct task *task, file_node_t *node, int flags );

void set_fs_root( file_node_t * );

#endif
