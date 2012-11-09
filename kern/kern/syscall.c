/** System call
 * necessary code for syscalls
 */ 
#ifndef _kernel_syscall_c
#define _kernel_syscall_c
#include <syscall.h>

static void syscall_handler( registers_t *regs );

#define NUM_SYSCALLS 20
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
	&fdopendir,
	&readdir,
	&mkdir,
	&chdir,

	&chroot,
	&mount,
	&unmount,
	&getpid,
	&fexecve,

	&create_thread,
	&send_msg,
	&get_msg,
	&kputs,
	&load_module
};

DEFN_SYSCALL0(	cls, 		0					);
DEFN_SYSCALL1(	exit,		1, 	char				);
DEFN_SYSCALL2(	open, 		2, 	char *, int			);
DEFN_SYSCALL1(	close,	 	3, 	int				);
DEFN_SYSCALL3(	read,		4, 	int, void *, unsigned long 	);	

DEFN_SYSCALL3(	write,		5, 	int, void *, unsigned long 	);	
DEFN_SYSCALL1(	fdopendir,	6, 	int				);	
DEFN_SYSCALL2(	readdir,	7, 	int, struct dirp * 		);	
DEFN_SYSCALL2(	mkdir,		8, 	char *, int			);
DEFN_SYSCALL1(	chdir, 		9,	char *				);

DEFN_SYSCALL1(	chroot,		10, 	char *				);
DEFN_SYSCALL4(	mount,		11,	char *, char *, int, void *	);
DEFN_SYSCALL2(	unmount,	12,	char *, int			);
DEFN_SYSCALL0(	getpid,		13					);
DEFN_SYSCALL3(	fexecve,	14, 	int, char **, char ** 		);

DEFN_SYSCALL1(	thread,		15,	void *	 			);
DEFN_SYSCALL2(	send_msg,	16,	unsigned long, ipc_msg_t * 	);
DEFN_SYSCALL2(	get_msg,	17,	unsigned long, ipc_msg_t * 	);
DEFN_SYSCALL1(	kputs, 		18,	char *				);
DEFN_SYSCALL2(	load_module,	19,	char *, int		 	);

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
