#ifndef _kernel_fs_h
#define _kernel_fs_h
#define MAX_NAME_LEN	256
#define MAX_DIRS 	256
#define MAX_INODES	64

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

struct dirent {
	unsigned char 	name[ MAX_NAME_LEN ];
	unsigned long	inode;
};

struct dirp {
	struct dirent *dir[ MAX_DIRS ];
	unsigned long dir_count, dir_ptr;
};

struct vfs_stat {
	file_type_t 	type;

	unsigned long	uid;
	unsigned long	gid;
	unsigned long 	time;
	unsigned long 	size;
	unsigned long	mask;
};

int open( char *path, int flags );
int close( int fd );
int read( int fd, void *buf, unsigned long size );
int write( int fd, void *buf, unsigned long size );
struct dirp *fdopendir( int fd );
struct dirent *readdir( int fd, struct dirp *dir );
int mkdir( char *path, int mode );
int chdir( char *path );
int chroot( char *path );
int lseek( int fd, long offset, int whence );
int lstat( char *path, struct vfs_stat *buf );
int mount( char *type, char *dir, int flags, void *data );
int unmount( char *dir, int flags );

#endif
