#ifndef _kernel_fs_h
#define _kernel_fs_h

#define MAX_NAME_LEN	256

#include <alloc.h>
#include <stdio.h>
#include <string.h>
#include <kmacros.h>
#include <task.h>
#include <stdint.h>
#include <errno.h>
#include <llist.h>

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

typedef enum {
	FOP_NULL,

	FOP_SET,
	FOP_GET,
} file_op_t;

typedef enum {
	FVAR_MOUNT_ID,
} file_var_t;

struct file_node;
struct file_info;
struct dirent;
struct task;

typedef int (*fs_call_func)( struct file_node *node, int op, int var, int value );
typedef int (*fs_get_info)( struct file_node *node, struct file_info *buf );

typedef int (*open_func)( struct file_node *, char *, int);
typedef int (*write_func)( struct file_node *, void *, unsigned long );
typedef int (*read_func)( struct file_node *,  void *, unsigned long );
typedef int (*pwrite_func)( struct file_node *, void *, unsigned long, unsigned long );
typedef int (*pread_func)( struct file_node *, void *, unsigned long, unsigned long );
typedef int (*ioctl_func)( struct file_node *, unsigned long, ... );
typedef struct dirp *(*opendir_func)( struct file_node * );
typedef int (*closedir_func)( struct file_node * );
typedef int (*close_func)( struct file_node * );

typedef int (*mkdir_func)( struct file_node *, char *, int );
typedef int (*mknod_func)( struct file_node *, char *, int, int );
typedef int (*link_func)( struct file_node *, struct file_node * );
typedef int (*unlink_func)( struct file_node * );
typedef int (*getdents_func)( struct file_node *, struct dirent *dirp, unsigned long count, unsigned long offset );
typedef int (*readdir_func)( struct file_node *, struct dirent *dirp, int entry );

typedef struct file_node *(*find_node_func)( struct file_node *, char *name, unsigned int links );

typedef struct file_functions {
	read_func  	read;
	pread_func  	pread;
	write_func	write;
	pwrite_func 	pwrite;
	ioctl_func  	ioctl;

	mkdir_func	mkdir;
	mknod_func	mknod;
	link_func	link;
	unlink_func	unlink;
	//getdents_func	getdents;
	readdir_func	readdir;

	open_func	open;
	close_func	close;

	fs_get_info	get_info;
	fs_call_func	callback;
} file_funcs_t;

/*! \brief File system descriptor, holds info on each file system available. */
typedef struct file_system {
	char *name;
	int id;

	file_funcs_t 	*ops;
	unsigned long 	i_root;
	//struct file_node *root;
	
	void *fs_data;
} file_system_t;

/*! \brief The internal file node structure, every file has one. */
typedef struct file_node { 
	unsigned long 	inode;

	struct file_system *fs;
} file_node_t;

typedef struct file_descript {
	file_node_t *file;
	unsigned long r_offset; /* read offset */
	unsigned long w_offset; /* write offset */
	unsigned long d_offset; /* directory offset */
} file_descript_t;

struct dirent {
	uint32_t inode;
	uint16_t size;
	uint16_t name_len;
	char 	 name[MAX_NAME_LEN];
};

typedef struct file_info {
	file_type_t	type;

	//char		name[ MAX_NAME_LEN ];
	char		*name;
	unsigned long 	mask;
	unsigned long	uid;
	unsigned long 	gid;
	unsigned long	time;
	unsigned long	inode;
	unsigned long	size;
	unsigned long	links;
	unsigned long 	flags;
	unsigned long	dev_id;

	file_system_t	*fs;

	unsigned long 	mount_id;

} file_info_t;

struct vfs_stat {
	file_type_t 	type;

	unsigned long	uid;
	unsigned long	gid;
	unsigned long 	time;
	unsigned long 	size;
	unsigned long	mask;
};


/* Some generic functions for acting on fs nodes */
/*
int  fs_write( file_node_t *, void *, unsigned long );
int   fs_read( file_node_t *, void *, unsigned long );
int fs_pwrite( file_node_t *, void *, unsigned long, unsigned long );
int  fs_pread( file_node_t *, void *, unsigned long, unsigned long );
int  fs_ioctl( file_node_t *, unsigned long, ... );
int   fs_open( file_node_t *, char *, int );
int  fs_close( file_node_t * );
int   fs_stat( file_node_t *, struct vfs_stat * );
int fs_unmount( file_node_t *dir, int flags );
file_node_t *fs_find_node( file_node_t *, char *, unsigned int );
*/
/*
struct dirp *fs_opendir( file_node_t * );
int fs_closedir( file_node_t * );
int fs_mkdir( file_node_t *, char *name, unsigned long );
int fs_mknod( file_node_t *, char *name, int mode, int dev );
int fs_unlink( file_node_t * );
*/

//struct dirent *fs_readdir_c( struct dirp *dir, struct dirent *buf ); 
int fs_mount( file_node_t *type, file_node_t *dir, int flags, void *data );
int fs_find_path( char *path, unsigned int links, file_node_t *buf );
int fs_call( file_op_t op, file_node_t *node, ... );

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
int unlink( char *path );
//int getdents( int fd, struct dirent *dirp, unsigned int count );
int readdir( int fd, struct dirent *dirp );

struct dirp *vfs_opendir( file_node_t * );
int vfs_closedir( file_node_t * );

int isgoodfd( struct task *task, int fd );
int check_perms( struct task *task, file_node_t *node, int flags );

void set_fs_root( file_system_t * );
int register_fs( file_system_t *newfs, int flags );

int register_mount_node( file_node_t *node );
int get_mount_node( int id, file_node_t *nodebuf );

int fs_find_node( file_node_t *node, char *path, int links, file_node_t *nodebuf );

#endif
