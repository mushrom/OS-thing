#ifndef _kernel_signal_h
#define _kernel_signal_h

typedef int (*ksignal_h)( int );

typedef enum {
	SIGABRT,
	SIGALRM,
	SIGBUS,
	SIGCHLD,
	SIGCONT,
	SIGFPE,
	SIGHUP,
	SIGILL,
	SIGINT,
	SIGKILL,
	SIGPIPE,
	SIGQUIT,
	SIGSEGV,
	SIGSTOP,
	SIGTERM,
	SIGTSTP,
	SIGTTIN,
	SIGTTOU,
	SIGUSR1,
	SIGUSR2,
	SIGPOLL,
	SIGPROF,
	SIGSYS,
	SIGTRAP,
	SIGURG,
	SIGVTALRM,
	SIGXCPU,
	SIGXFSZ
} signal_t;

#include <task.h>

struct task;
int handle_signal( struct task *task, signal_t signal );
ksignal_h signal( signal_t signal, ksignal_h handler );
int kill( int pid, signal_t signal );
int sigreturn( int unused );
void jmp_to_signal( unsigned long sig, ksignal_h handle );

#endif
