#ifndef _user_syscall_c
#define _user_syscall_c
#include <syscall.h>

DEFN_SYSCALL0(	cls,		0 );
DEFN_SYSCALL1(	exit, 		1, char);
DEFN_SYSCALL2(	open, 		2, char *, int );
DEFN_SYSCALL1(	close, 		3, int );
DEFN_SYSCALL3(	read, 		4, int, void *, unsigned long );

DEFN_SYSCALL3(	write, 		5, int, void *, unsigned long );
DEFN_SYSCALL3(	lseek, 		6, int, long, int );
DEFN_SYSCALL2(	lstat,		7, int, struct vfs_stat * );
DEFN_SYSCALL2(	fdopendir_c,	8, int, struct dirp * );
DEFN_SYSCALL3(	readdir_c,	9, int, struct dirp *, struct dirent * ); 
DEFN_SYSCALL2(	mkdir,		10, char *, int );
DEFN_SYSCALL1(	chdir, 		11, char *);

DEFN_SYSCALL1(	chroot, 	12, char *);
DEFN_SYSCALL4(	mount, 		13, char *, char *, int, void * );
DEFN_SYSCALL2(	unmount, 	14, char *, int );
DEFN_SYSCALL0(	getpid, 	15 );
DEFN_SYSCALL3(	fspawn, 	16, int, char **, char ** );

DEFN_SYSCALL1(	thread, 	17, void *);
DEFN_SYSCALL1(	wait,		18, int * );
DEFN_SYSCALL1(	kill,		19, unsigned long );
DEFN_SYSCALL2(	send_msg, 	20, unsigned long, ipc_msg_t *);
DEFN_SYSCALL2(	get_msg, 	21, unsigned long, ipc_msg_t *);
DEFN_SYSCALL1(	kputs, 		22, char *);
DEFN_SYSCALL2(	load_module, 	23, char *, int );

#endif
