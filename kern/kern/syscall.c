/** System call
 * necessary code for syscalls
 */ 
#ifndef _kernel_syscall_c
#define _kernel_syscall_c
#include <syscall.h>

static void syscall_handler( registers_t *regs );

#define NUM_SYSCALLS 26
/** System call table 
 * Contains the address of functions to jump to
 * upon recieving int 0x50
 */
static void *syscalls[ NUM_SYSCALLS ] = {
	&cls,
	&exit,
	&open,
	&close,
	&read,

	&write,
	&lseek,
	&lstat,
	&fdopendir_c,
	&readdir_c,

	&mkdir,
	&chdir,
	&chroot,
	&mount,
	&unmount,

	&getpid,
	&fexecve,
	&create_thread,
	&wait,
	&kill_thread,

	&send_msg,
	&get_msg,
	&kputs,
	&load_module,
	&kexport_symbol,

	&kget_symbol
};

DEFN_SYSCALL0(	cls, 		0					);
DEFN_SYSCALL1(	exit,		1, 	char				);
DEFN_SYSCALL2(	open, 		2, 	char *, int			);
DEFN_SYSCALL1(	close,	 	3, 	int				);
DEFN_SYSCALL3(	read,		4, 	int, void *, unsigned long 	);	

DEFN_SYSCALL3(	write,		5, 	int, void *, unsigned long 	);	
DEFN_SYSCALL3(	lseek,		6,	int, long, int			);
DEFN_SYSCALL2(	lstat,		7,	int, struct vfs_stat *		);
DEFN_SYSCALL2(	fdopendir_c,	8, 	int, struct dirp *		);	
DEFN_SYSCALL3(	readdir_c,	9, 	int, struct dirp *, struct dirent * );	

DEFN_SYSCALL2(	mkdir,		10, 	char *, int			);
DEFN_SYSCALL1(	chdir, 		11,	char *				);
DEFN_SYSCALL1(	chroot,		12, 	char *				);
DEFN_SYSCALL4(	mount,		13,	char *, char *, int, void *	);
DEFN_SYSCALL2(	unmount,	14,	char *, int			);

DEFN_SYSCALL0(	getpid,		15					);
DEFN_SYSCALL3(	fexecve,	16, 	int, char **, char ** 		);
DEFN_SYSCALL1(	thread,		17,	void *	 			);
DEFN_SYSCALL1(	wait,		18,	int *				);
DEFN_SYSCALL1(	kill,		19,	unsigned long			);

DEFN_SYSCALL2(	send_msg,	20,	unsigned long, ipc_msg_t * 	);
DEFN_SYSCALL2(	get_msg,	21,	unsigned long, ipc_msg_t * 	);
DEFN_SYSCALL1(	kputs, 		22, 	char *				);
DEFN_SYSCALL2(	load_module,	23,	char *, int		 	);
DEFN_SYSCALL2(	kexport_symbol,	24,	char *, unsigned long		);

DEFN_SYSCALL1(	kget_symbol,	25,	char * 				);

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
