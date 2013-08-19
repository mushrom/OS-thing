/** System call
 * necessary code for syscalls
 */ 
#ifndef _kernel_syscall_c
#define _kernel_syscall_c
#include <syscall.h>

static void syscall_handler( registers_t *regs );

/** System call table 
 * Contains the address of functions to jump to
 * upon recieving int 0x50
 */
static void *syscalls[] = {
	&cls,
	&exit,
	&open,
	&close,
	&read,

	&write,
	&lseek,
	&lstat,
	/*
	&fdopendir_c,
	&readdir_c,
	&getdents,
	*/
	&readdir,
	&mkdir,
	&chdir,

	&chroot,
	&mount,
	&unmount,
	&getpid,
	&fexecve,
	&create_thread,

	&wait,
	&kill,
	&send_msg,
	&get_msg,
	&kputs,

	&load_module,
	&kexport_symbol,
	&kget_symbol,
	&sbrk,
	&signal
};
#define NUM_SYSCALLS sizeof( syscalls )

DEFN_SYSCALL0(	cls, 		0					);
DEFN_SYSCALL1(	exit,		1, 	char				);
DEFN_SYSCALL2(	open, 		2, 	char *, int			);
DEFN_SYSCALL1(	close,	 	3, 	int				);
DEFN_SYSCALL3(	read,		4, 	int, void *, unsigned long 	);	

DEFN_SYSCALL3(	write,		5, 	int, void *, unsigned long 	);	
DEFN_SYSCALL3(	lseek,		6,	int, long, int			);
DEFN_SYSCALL2(	lstat,		7,	char *, struct vfs_stat *	);
DEFN_SYSCALL3(	getdents,	8,	int, struct dirp *, unsigned long );

DEFN_SYSCALL2(	mkdir,		9, 	char *, int			);
DEFN_SYSCALL1(	chdir, 		10,	char *				);
DEFN_SYSCALL1(	chroot,		11, 	char *				);
DEFN_SYSCALL4(	mount,		12,	char *, char *, int, void *	);
DEFN_SYSCALL2(	unmount,	13,	char *, int			);

DEFN_SYSCALL0(	getpid,		14					);
DEFN_SYSCALL3(	fexecve,	15, 	int, char **, char ** 		);
DEFN_SYSCALL1(	thread,		16,	void *	 			);
DEFN_SYSCALL1(	wait,		17,	int *				);
DEFN_SYSCALL2(	kill,		18,	int, int			);

DEFN_SYSCALL2(	send_msg,	19,	unsigned long, ipc_msg_t * 	);
DEFN_SYSCALL2(	get_msg,	20,	unsigned long, ipc_msg_t * 	);
DEFN_SYSCALL1(	kputs, 		21, 	char *				);
DEFN_SYSCALL2(	load_module,	22,	char *, int		 	);
DEFN_SYSCALL2(	kexport_symbol,	23,	char *, unsigned long		);

DEFN_SYSCALL1(	kget_symbol,	24,	char * 				);
DEFN_SYSCALL1(	sbrk,	 	25, 	int				);
DEFN_SYSCALL2(	signal,		26,	int, void *			);
DEFN_SYSCALL1(	sigreturn,	27,	int				);

void init_syscalls(){
	register_interrupt_handler( 0x50, &syscall_handler );
}

/** System call isr handler
 * @param regs Registers pushed from isr_common_stub
 */
static void syscall_handler( registers_t *regs ){
	if ( regs->eax >= NUM_SYSCALLS )
		return;

	void *location = syscalls[regs->eax];

	int ret;
	asm volatile( "	\
	push %1;	\
	push %2;	\
	push %3;	\
	push %4;	\
	push %5;	\
	call *%6;	\
	pop %%ebx;	\
	pop %%ebx;	\
	pop %%ebx;	\
	pop %%ebx;	\
	pop %%ebx;	\
	" : "=a"(ret) : "r"(regs->edi), "r"(regs->esi), "r"(regs->edx), "r"(regs->ecx), "r"(regs->ebx), "r"(location));
	regs->eax = ret;
}

#endif
