#ifndef _kernel_task_h
#define _kernel_task_h

#include <mem/paging.h>
#include <mem/alloc.h>
#include <lib/kmacros.h>
#define KERNEL_STACK_SIZE 2048

struct page_dir;

typedef struct task {
	unsigned long 	eip,
			esp,
			ebp,
			id;
	struct page_dir *dir;
	unsigned long stack;
	unsigned long sleep;
	struct task *next;
	struct task *prev;
	struct task *parent;
} task_t;

void move_stack( void *, unsigned long size );
void create_thread( void (*)());
void exit_thread( );
void init_tasking( );
void dump_pids( void );
int  getpid( void );

#endif
