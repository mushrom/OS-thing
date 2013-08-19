#ifndef _kernel_signal_c
#define _kernel_signal_c
#include <signal.h>

extern task_t *current_task;

// TODO:
// 	Get signals working
void jmp_to_signal( unsigned long sig, ksignal_h handle ){
	asm volatile( "		\
		cli;		\
		mov %1, %%esp;	\
		pushl $1;	\
		pushl %%eax;	\
		pushl $1;	\
		pushl $2;	\
		pushl $3;	\
		pushl $4;	\
		pushl $5;	\
				\
		mov $0x23, %%ax;\
		mov %%ax, %%ds;	\
		mov %%ax, %%es;	\
		mov %%ax, %%fs;	\
		mov %%ax, %%gs;	\
				\
		mov %%esp, %%eax;\
		pushl $0x23;	\
		pushl %%eax;	\
		pushf;		\
		popl %%eax;	\
		orl $0x200, %%eax;\
		pushl %%eax;	\
		pushl $0x1b;	\
		pushl %0;	\
		iret;		\
	":: "m"( handle ), "r"( current_task->esp ));
	//printf( "[kern] jumping to signal at 0x%x\n", handle );
	//switch_to_usermode_jmp( handle );
	//asm volatile( "sti" );
}

int handle_signal( task_t *task, signal_t signal ){
	printf( "[kernel] sent signal %d\n", signal );
	if ( signal < 0 || signal >= MAX_SIGNALS )
		return -1;

	if ( task->sig_queued )
		return -1;

	if ( !task->sighandle[ signal ] ){
		remove_task( task );
		return 0;
		// exit task
	}

	//int val = (int)signal;

	task->sig_queued = true;
	task->signal = signal;

	if ( current_task == task )
		sleep_thread( 1 ); 
	//PUSH( task->esp, task->eip );
	//PUSH( task->esp, val );

	//task->eip = task->sighandle[ signal ];

	return 0;
}

ksignal_h signal( signal_t signal, ksignal_h handler ){
	if ( signal < 0 || signal >= MAX_SIGNALS )
		return 0;
	
	current_task->sighandle[ signal ] = handler;

	return handler;
}

int sigreturn( int unused ){
	asm volatile( "cli" );

	printf( "Got here\n" );
	printf( "in signal: %d\n", current_task->in_sig );
	/*
	if ( !current_task->in_sig )
		return -1;
	*/

	current_task->in_sig = false;
	printf( "Got here: eip: 0x%x, old_eip: 0x%x\n", current_task->eip, 
		current_task->old_eip );
	current_task->eip = current_task->old_eip;
	asm volatile( "popl %eax" );
	asm volatile( "sti" );
	switch_task();
	return 0;
}

int kill( int pid, signal_t signal ){
	task_t *p = get_pid_task( pid );

	if ( !p )
		return -1;

	return handle_signal( p, signal );
}

#endif
