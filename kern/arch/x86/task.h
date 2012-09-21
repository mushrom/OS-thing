#ifndef _kernel_task_h
#define _kernel_task_h
#include <arch/x86/paging.h>
#include <lib/stdint.h>
#include <lib/stdio.h>
#include <lib/kmacros.h>

typedef struct task {
	int id;
	uint32_t esp, ebp;
	uint32_t eip;
	page_directory_t *page_directory;
	struct task *next;
} task_t;

void init_tasking();
void switch_task();
void move_stack( void *, uint32_t );
void dump_pids();
void test_point();
void switch_to_user_mode();
int  fork();
int  getpid();

#endif
