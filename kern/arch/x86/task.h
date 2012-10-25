#ifndef _kernel_task_h
#define _kernel_task_h

#include <mem/paging.h>
#include <mem/alloc.h>
#include <lib/kmacros.h>
#include <kern/ipc.h>
#define KERNEL_STACK_SIZE 4096
#define MAX_MSGS 32

enum {
	S_RUNNING,
	S_SLEEPING,
	S_LISTENING,
	S_SENDING
};

struct page_dir;

typedef struct task {
	unsigned long 	eip,
			esp,
			ebp,
			id;
	unsigned long stack;
	unsigned long sleep;
	unsigned long status;
	unsigned long time;
	ipc_msg_t *msg_buf[MAX_MSGS];
	unsigned char msg_ptr;
	unsigned long msg_count;
	struct page_dir *dir;
	struct task *next;
	struct task *prev;
	struct task *parent;
} task_t;

void init_tasking( );
int  create_thread( void (*)());
void sleep_thread( unsigned long time );
void exit_thread( );

void gen_msg( ipc_msg_t *msg, unsigned char type, unsigned long size, void *buf );
int  send_msg( unsigned long pid, ipc_msg_t *msg );
int  get_msg( ipc_msg_t *buf, int blocking );

void dump_pids( void );
int  getpid( void );
void switch_to_usermode( void );
void switch_task();
void move_stack( void *, unsigned long size );

task_t *get_pid_task( unsigned long pid );

#endif
