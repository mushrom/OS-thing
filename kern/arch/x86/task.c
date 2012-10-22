#ifndef _kernel_task_c
#define _kernel_task_c
#include <arch/x86/task.h>

extern page_dir_t *current_dir;
extern unsigned long initial_esp;
extern unsigned long read_eip();
volatile task_t *current_task = 0,
		*task_queue = 0;

unsigned long next_pid = 1;

void init_tasking( ){
	asm volatile( "cli" );

	current_task = task_queue = (task_t *)kmalloc( sizeof( task_t ), 0, 0 );
	current_task->id = next_pid++;
	current_task->esp = current_task->ebp = 0;
	current_task->eip = 0;
	current_task->dir = current_dir;
	current_task->next = 0;
	current_task->stack = kmalloc( KERNEL_STACK_SIZE, 1, 0 );
	current_task->sleep = 0;

	asm volatile( "sti" );
}

void switch_task(){
	if ( !current_task )
		return;

	//printf( "[1]"  );
	unsigned long esp = 0, ebp = 0, eip = 0;
	asm volatile( "mov %%esp, %0" : "=r"(esp));
	asm volatile( "mov %%ebp, %0" : "=r"(ebp));
	eip = read_eip();
	//printf( "[2] 0x%x ", eip );

	if ( eip == 0xdeadbeef )
		return;

	//printf( "[3] "  );
	current_task->eip = eip;
	current_task->esp = esp;
	current_task->ebp = ebp;

	current_task = current_task->next;
	if ( !current_task ) current_task = task_queue;

	eip = current_task->eip;
	esp = current_task->esp;
	ebp = current_task->ebp;

	current_dir = current_task->dir;
	set_kernel_stack( current_task->stack + KERNEL_STACK_SIZE );

	//printf( "[4] 0x%x\n", eip );

	asm volatile (" 	\
		cli;		\
		mov %0, %%ecx;	\
		mov %1, %%esp;	\
		mov %2, %%ebp;	\
		mov %3, %%cr3;	\
		mov $0xdeadbeef, %%eax;\
		sti;\
		jmp *%%ecx" :: "r"(eip), "r"(esp), "r"(ebp), "r"(current_dir->address ));
}

void create_thread( void (*function)()){
	asm volatile( "cli" );

	//unsigned long eip = 0;
	task_t 	*parent = (task_t *)current_task,
		*temp	= (task_t *)task_queue;

	task_t *new_task = (task_t *)kmalloc( sizeof( task_t ), 0, 0 );
	new_task->id = next_pid++;
	new_task->esp = new_task->ebp = new_task->eip = 0;
	new_task->dir = current_dir;
	new_task->next = 0;
	new_task->stack = kmalloc( KERNEL_STACK_SIZE, 1, 0 );
	new_task->parent = parent;
	new_task->eip = (unsigned long)function;
	new_task->esp = new_task->stack;
	new_task->ebp = current_task->ebp;

	temp = (task_t *)task_queue;
	while ( temp->next )
		temp = temp->next;
	temp->next = new_task;

	asm volatile( "sti" );
}
	
void exit_thread( ){
	asm volatile( "cli" );
	task_t *temp = (task_t *)current_task;
	if ( temp->next ){
		temp->next->parent = temp->parent;
		temp->parent->next = temp->next;
	} else {
		temp->parent->next = 0;
	}
	printf( "pid %d exited\n", temp->id );
	switch_task();
	asm volatile( "sti" );
}

void move_stack( void *new_stack_start, unsigned long size ){
	unsigned long i;
	for ( i = (unsigned long)new_stack_start; i >= (unsigned long)new_stack_start - size; i -= 0x1000 ){
		alloc_page( get_page( i, 1, current_dir ));
	}
	for ( i = (unsigned long)new_stack_start; i >= (unsigned long)new_stack_start - size; i -= 0x1000 ){
		set_table_perms( PAGE_WRITEABLE | PAGE_PRESENT, i, current_dir );
	}
	unsigned long pd_addr;
	asm volatile( "mov %%cr3, %0" : "=r" (pd_addr));
	asm volatile( "mov %0, %%cr3" :: "r" (pd_addr));
	//memcpy((char *)0xe0000000 - 0x1000, "test", 5 );
	printf( "    Woot, it didn't break. :D\n", 0xe0000000 - 0x1000 );

	unsigned long old_esp, old_ebp, offset, new_esp, new_ebp;
	asm volatile( "mov %%esp, %0" : "=r" (old_esp));
	asm volatile( "mov %%ebp, %0" : "=r" (old_ebp));

	offset = (unsigned long)new_stack_start - initial_esp;
	new_esp = old_esp + offset;
	//new_ebp = old_ebp + offset;
	memcpy((void *)new_esp, (void *)old_esp, initial_esp - old_esp );
}

int getpid(){
	return current_task->id;
}

void dump_pids( void ){
	task_t *temp = (task_t *)task_queue;
	while ( temp ){
		printf( "pid: %d, state: %s\n", temp->id, (temp->sleep)?"sleeping":"running" );
		temp = temp->next;
	}
	temp = (task_t *)task_queue;
}

#endif
