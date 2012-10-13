#ifndef _kernel_syscall_c
#define _kernel_syscall_c
#include <sys/syscall.h>

static void syscall_handler( registers_t regs );

#define NUM_SYSCALLS 3
static void *syscalls[ NUM_SYSCALLS ] = {
	&kputchar,
	&kputs,
	&cls
};

DEFN_SYSCALL0(cls, 0)
DEFN_SYSCALL1(kputchar, 1, char)
DEFN_SYSCALL1(kputs, 1, char *);

void init_syscalls(){
	register_interrupt_handler( 0x50, &syscall_handler );
}

static void syscall_handler( registers_t regs ){
	if ( regs.eax >= NUM_SYSCALLS )
		return;

	void *location = syscalls[regs.eax];

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
	" : "=a"(ret) : "r"(regs.edi), "r"(regs.esi), "r"(regs.edx), "r"(regs.ecx), "r"(regs.ebx), 
		"r"(location));
	regs.eax = ret;

}

#endif
