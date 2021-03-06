#ifndef _user_syscall_h
#define _user_syscall_h
#include <ipc.h>
#include <fs.h>

#define DECL_SYSCALL0(fn) int syscall_##fn();
#define DECL_SYSCALL1(fn, p1) int syscall_##fn(p1);
#define DECL_SYSCALL2(fn, p1, p2) int syscall_##fn(p1,p2);
#define DECL_SYSCALL3(fn, p1, p2, p3) int syscall_##fn(p1,p2,p3);
#define DECL_SYSCALL4(fn, p1, p2, p3, p4) int syscall_$$fn(p1,p2,p3,p4);
#define DECL_SYSCALL5(fn, p1, p2, p3, p4, p5) int syscall_$$fn(p1,p2,p3,p4,p5);

#define DEFN_SYSCALL0(fn, num) \
int syscall_##fn(){ \
	int a;\
	asm volatile( "int $0x50" : "=a" (a) : "0" (num));\
	return a;\
}

#define DEFN_SYSCALL1(fn, num, P1) \
int syscall_##fn(P1 p1){ \
	int a; \
	asm volatile( "int $0x50" : "=a" (a) : "0" (num), "b"((int)p1)); \
	return a; \
}

#define DEFN_SYSCALL2(fn, num, P1, P2) \
int syscall_##fn(P1 p1, P2 p2){ \
	int a; \
	asm volatile( "int $0x50" : "=a" (a) : "0" (num), "b"((int)p1), "c"((int)p2)); \
	return a; \
}

#define DEFN_SYSCALL3(fn, num, P1, P2, P3) \
int syscall_##fn(P1 p1, P2 p2, P3 p3){ \
	int a; \
	asm volatile( "int $0x50" : "=a" (a) : "0" (num), "b"((int)p1), "c"((int)p2), "d"((int)p3)); \
	return a; \
}

#define DEFN_SYSCALL4(fn, num, P1, P2, P3, P4) \
int syscall_##fn(P1 p1, P2 p2, P3 p3, P4 p4){ \
	int a; \
	asm volatile( "int $0x50" : "=a" (a) : "0" (num), "b"((int)p1), "c"((int)p2), "d"((int)p3), "S"((int)p4)); \
	return a; \
}

struct dirp;

DECL_SYSCALL0(	cls 			);
DECL_SYSCALL1(	exit, 	char		);
DECL_SYSCALL2(	open, 	char *, int 	);
DECL_SYSCALL1(	close, 	int 		);
DECL_SYSCALL3(	read, 	int, void *, unsigned long );

DECL_SYSCALL3(	write, 	int, void *, unsigned long );
DECL_SYSCALL3(	lseek,  int, long, int );
DECL_SYSCALL2(	lstat,	char *, struct vfs_stat * );
DECL_SYSCALL2(	fdopendir_c, int, struct dirp * );
DECL_SYSCALL3(	readdir_c, int, struct dirp *, struct dirent * ); 

DECL_SYSCALL2(	mkdir, char *, int );
DECL_SYSCALL1(	chdir,  char *);
DECL_SYSCALL1(	chroot, char *);
DECL_SYSCALL4(	mount, char *, char *, int, void * );
DECL_SYSCALL2(	unmount, char *, int );

DECL_SYSCALL0(	getpid );
DECL_SYSCALL3(	fspawn, int, char **, char ** );
DECL_SYSCALL1(	thread, void *);
DECL_SYSCALL1(	wait, int * );
DECL_SYSCALL2(	kill, int, int );

DECL_SYSCALL2(	send_msg, unsigned long, ipc_msg_t *);
DECL_SYSCALL2(	get_msg, unsigned long, ipc_msg_t *);
DECL_SYSCALL1(	kputs, char *);
DECL_SYSCALL2(	load_module, char *, int );
DECL_SYSCALL1(	sbrk, 	int 		);

DECL_SYSCALL2(	signal,	int, void * );
DECL_SYSCALL1(	sigreturn, int );

#define exit	syscall_exit
#define open	syscall_open
#define close	syscall_close
#define read	syscall_read
#define write	syscall_write
#define mkdir	syscall_mkdir
#define chdir	syscall_chdir
#define chroot	syscall_chroot
#define mount	syscall_mount
#define unmount	syscall_unmount
#define getpid	syscall_getpid
#define fspawn  syscall_fspawn
#define kill	syscall_kill
#define wait	syscall_wait
#define fdopendir_c syscall_fdopendir_c
#define readdir_c syscall_readdir_c
#define sbrk	(void *)syscall_sbrk
#define signal	syscall_signal
#define sigreturn syscall_sigreturn

#endif
