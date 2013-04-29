#ifndef _kernel_syscall_h
#define _kernel_syscall_h
#include <isr.h>
#include <stdio.h>
#include <task.h>
#include <fs.h>
#include <module.h>

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
	asm volatile( "int $0x50" : "=a" (a) : "0" (num), "b"((int)p1), "c"((int)p2), "d"((int)p3), "S"((int)p4));\
	return a; \
}

DECL_SYSCALL0(cls)
DECL_SYSCALL1(exit, char)
DECL_SYSCALL2(open, char *, int );
DECL_SYSCALL1(close, int );
DECL_SYSCALL3(read, int, void *, unsigned long );
DECL_SYSCALL3(write, int, void *, unsigned long );

DECL_SYSCALL2(fdopendir_c, int, struct dirp * )
DECL_SYSCALL3(readdir_c, int, struct dirp *, struct dirent * );
DECL_SYSCALL1(chdir, char *);

DECL_SYSCALL0(getpid)
DECL_SYSCALL3(fexecve, int, char **, char ** );
DECL_SYSCALL1(thread, void *);

DECL_SYSCALL1(kputs, char *);

void init_syscalls();

#endif
