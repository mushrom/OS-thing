#ifndef _kernel_task_h
#define _kernel_task_h
#include <arch/x86/paging.h>
#include <arch/x86/init_tables.h>
#include <lib/stdint.h>
#include <lib/stdio.h>
#include <lib/kmacros.h>
#define KERNEL_STACK_SIZE 2048

typedef struct task {
	int id;
	uint32_t esp, ebp;
	uint32_t eip;
	page_directory_t *page_directory;
	uint32_t kernel_stack;
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
