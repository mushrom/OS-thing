/** System call
 * necessary code for syscalls
 */ 
#ifndef _kernel_syscall_c
#define _kernel_syscall_c
#include <syscall.h>

static void syscall_handler( registers_t *regs );

#define NUM_SYSCALLS 9
/** System call table 
 * Contains the address of functions to jump to
 * upon recieving int 0x50
 */
static void *syscalls[ NUM_SYSCALLS ] = {
	&cls,
	&open,
	&close,
	&read,
	&write,

	&fdopendir,
	&readdir,
	&getpid,
	&kputs
};

DEFN_SYSCALL0(	cls, 	0		);
DEFN_SYSCALL2(	open, 	1, char *, int	);
DEFN_SYSCALL1(	close, 	2, int		);
DEFN_SYSCALL3(	read,	3, int,	void *, unsigned long );	
DEFN_SYSCALL3(	write,	4, int,	void *, unsigned long );	
DEFN_SYSCALL1(fdopendir,5, int );	
DEFN_SYSCALL2(	readdir,6, int, struct dirp * );	
DEFN_SYSCALL0(	getpid,	7		);
DEFN_SYSCALL1(	kputs, 	8, char *	);

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
