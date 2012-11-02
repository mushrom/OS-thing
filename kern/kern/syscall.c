/** System call
 * necessary code for syscalls
 */ 
#ifndef _kernel_syscall_c
#define _kernel_syscall_c
#include <syscall.h>

static void syscall_handler( registers_t *regs );

#define NUM_SYSCALLS 15
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
	&chdir,
	&getpid,

	&fexecve,
	&create_thread,
	&send_msg,
	&get_msg,
	&kputs
};

DEFN_SYSCALL0(	cls, 		0					);
DEFN_SYSCALL1(	exit,		1, 	char				);
DEFN_SYSCALL2(	open, 		2, 	char *, int			);
DEFN_SYSCALL1(	close,	 	3, 	int				);
DEFN_SYSCALL3(	read,		4, 	int, void *, unsigned long 	);	
DEFN_SYSCALL3(	write,		5, 	int, void *, unsigned long 	);	
DEFN_SYSCALL1(	fdopendir,	6, 	int				);	
DEFN_SYSCALL2(	readdir,	7, 	int, struct dirp * 		);	
DEFN_SYSCALL1(	chdir, 		8,	char *				);
DEFN_SYSCALL0(	getpid,		9					);
DEFN_SYSCALL3(	fexecve,	10, 	int, char **, char ** 		);
DEFN_SYSCALL1(	thread,		11,	void *	 			);
DEFN_SYSCALL2(	send_msg,	12,	unsigned long, ipc_msg_t * 	);
DEFN_SYSCALL2(	get_msg,	13,	unsigned long, ipc_msg_t * 	);

DEFN_SYSCALL1(	kputs, 		14,	char *				);

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
