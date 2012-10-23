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
	current_task->time = 0;
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
	task_t *temp = (task_t *)current_task;
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

	temp = current_task->next;
	if ( !temp ) 
		temp = (task_t *)task_queue;

	while( temp->sleep ){
		if ( temp->sleep )
			temp->sleep--;
		temp = temp->next;
		if ( !temp )
			temp = (task_t *)task_queue;
	}
	current_task = temp;
	current_task->time++;
	current_task->status = S_RUNNING;

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

int create_thread( void (*function)()){
	asm volatile( "cli" );

	//unsigned long eip = 0;
	task_t 	*parent = (task_t *)current_task,
		*temp	= (task_t *)task_queue;

	task_t *new_task = (task_t *)kmalloc( sizeof( task_t ), 0, 0 );
	new_task->esp = new_task->ebp = new_task->eip = 0;

	new_task->id = next_pid++;
	new_task->next = 0;
	new_task->parent = parent;
	new_task->sleep = 0;
	new_task->time  = 0;
	new_task->dir = current_dir;
	new_task->stack = kmalloc( KERNEL_STACK_SIZE, 1, 0 );
	new_task->eip = (unsigned long)function;
	new_task->esp = new_task->stack;
	new_task->ebp = current_task->ebp;

	temp = (task_t *)task_queue;
	while ( temp->next ){
		parent = temp;
		temp = temp->next;
	}
	temp->next = new_task;
	temp->parent = parent;

	asm volatile( "sti" );
	return new_task->id;
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

int kill_thread( unsigned long pid ){
	asm volatile( "cli" );
	task_t *temp = get_pid_task( pid );

	if ( !temp ){
		asm volatile( "sti" );
		return 1;
	}
	temp->next->parent = temp->parent;
	temp->parent->next = temp->next;

	asm volatile( "sti" );
	return 0;
}

void sleep_thread( unsigned long time ){
	asm volatile( "cli" );

	current_task->sleep = time;
	current_task->status = S_SLEEPING;
	//switch_task();

	asm volatile( "sti" );
}

void get_msg( ipc_msg_t *buf ){
	asm volatile( "cli" );
	task_t *temp = (task_t *)current_task;
	asm volatile( "sti" );
	while ( !temp->msg_buf ){
		temp->status = S_LISTENING;
		//switch_task();
	}
	memcpy( buf, temp->msg_buf, sizeof( ipc_msg_t ));
	temp->msg_buf = 0;
}

int send_msg( unsigned long pid, ipc_msg_t *msg ){
	asm volatile( "cli" );
	task_t *temp = get_pid_task( pid );
	if ( !temp ){
		asm volatile( "sti" );
		return 1;
	}

	if ( temp->status != S_LISTENING ){
		asm volatile( "sti" );
		return 2;
	}

	/*
	while ( temp->status != S_LISTENING ){
		current_task->status = S_SENDING;
		switch_task();
	}
	*/

	temp->msg_buf = msg;
	asm volatile( "sti" );
	return 0;
}

task_t *get_pid_task( unsigned long pid ){
	task_t *temp = (task_t *)task_queue;
	while ( temp ){
		if ( temp->id == pid )
			return temp;
		temp = temp->next;
	}
	return 0;
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
	char *buf;
	printf( "pid:\tstate:\t\ttime:\n" );
	while ( temp ){
		printf( "%d ", temp->id );
		switch ( temp->status ){
			case S_RUNNING:
				buf = "running";
				break;
			case S_SLEEPING:
				buf = "sleeping";
				break;
			case S_LISTENING:
				buf = "listening";
				break;
			case S_SENDING:
				buf = "sending";
				break;
			default:
				buf = "unknown";
				break;
		}
		printf( "\t%s ", buf );
		printf( "\t%d\n", temp->time );
		temp = temp->next;
	}
	temp = (task_t *)task_queue;
}

void switch_to_usermode( void ){
	set_kernel_stack( current_task->stack + KERNEL_STACK_SIZE );

	asm volatile( "		\
		cli;		\
		mov $0x23, %ax;	\
		mov %ax, %ds;	\
		mov %ax, %es;	\
		mov %ax, %fs;	\
		mov %ax, %gs;	\
				\
		mov %esp, %eax;	\
		pushl $0x23;	\
		pushl %eax;	\
		pushf;		\
		popl %eax;	\
		or $0x200, %eax; \
		pushl %eax;	\
		pushl $0x1b;	\
		push $1f;	\
		iret;		\
		1:		\
	" );
}

#endif
