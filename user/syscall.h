#ifndef _user_syscall_h
#define _user_syscall_h
#include "ipc.h"

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

DECL_SYSCALL0(cls)
DECL_SYSCALL1(exit, char)
DECL_SYSCALL2(open, char *, int );
DECL_SYSCALL1(close, int );
DECL_SYSCALL3(read, int, void *, unsigned long );
DECL_SYSCALL3(write, int, void *, unsigned long );
DECL_SYSCALL1(fdopendir, int)
DECL_SYSCALL2(readdir, int, struct dirp * );
DECL_SYSCALL1(chdir, char *);
DECL_SYSCALL1(chroot, char *);
DECL_SYSCALL4(mount, char *, char *, int, void * );
DECL_SYSCALL2(unmount, char *, int );
DECL_SYSCALL0(getpid)
DECL_SYSCALL3(fexecve, int, char **, char ** );
DECL_SYSCALL1(thread, void *);
DECL_SYSCALL2(send_msg, unsigned long, ipc_msg_t *);
DECL_SYSCALL2(get_msg, unsigned long, ipc_msg_t *);
DECL_SYSCALL1(kputs, char *);

DEFN_SYSCALL0(cls, 	0 )
DEFN_SYSCALL1(exit, 	1, char)
DEFN_SYSCALL2(open, 	2, char *, int );
DEFN_SYSCALL1(close, 	3, int );
DEFN_SYSCALL3(read, 	4, int, void *, unsigned long );
DEFN_SYSCALL3(write, 	5, int, void *, unsigned long );
DEFN_SYSCALL1(fdopendir,6, int)
DEFN_SYSCALL2(readdir,	7, int, struct dirp * );
DEFN_SYSCALL2(mkdir,	8, char *, int );
DEFN_SYSCALL1(chdir, 	9, char *);
DEFN_SYSCALL1(chroot, 	10, char *);
DEFN_SYSCALL4(mount, 	11, char *, char *, int, void * );
DEFN_SYSCALL2(unmount, 	12, char *, int );
DEFN_SYSCALL0(getpid, 	13 )
DEFN_SYSCALL3(fexecve, 	14, int, char **, char ** );
DEFN_SYSCALL1(thread, 	15, void *);
DEFN_SYSCALL2(send_msg, 16, unsigned long, ipc_msg_t *);
DEFN_SYSCALL2(get_msg, 	17, unsigned long, ipc_msg_t *);
DEFN_SYSCALL1(kputs, 	18, char *);
DEFN_SYSCALL2(load_module, 19, char *, int );

#endif
