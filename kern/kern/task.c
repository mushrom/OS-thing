#ifndef _kernel_task_c
#define _kernel_task_c
#include <task.h>

extern page_dir_t *current_dir;
extern unsigned long initial_esp;
extern unsigned long read_eip(); 	/**< \brief Get current EIP */
extern unsigned long isr_error_count;
extern unsigned long get_tick();
extern void set_kernel_stack();
volatile task_t *current_task = 0,
		*task_queue = 0,
		*last_task = 0;

unsigned long next_pid = 1;

void init_tasking( ){
	asm volatile( "cli" );
	extern file_node_t *fs_root;

	last_task = current_task = task_queue = (task_t *)kmalloc( sizeof( task_t ), 0, 0 );

	current_task->id = next_pid++;
	current_task->parent = (task_t *)current_task;
	current_task->next = 0;

	current_task->dir = current_dir;
	current_task->stack = kmalloc( KERNEL_STACK_SIZE, 1, 0 );

	current_task->last_time = 0;
	current_task->time = 0;
	current_task->sleep = 0;

	current_task->child = 0;
	current_task->waiting = false;
	current_task->child_count = 0;

	current_task->esp = current_task->ebp = 0;
	current_task->eip = 0;

	current_task->cwd  = fs_root;
	current_task->root = fs_root;

	current_task->argv = 0;
	current_task->envp = 0;

	int i = 0;
	for ( i = 0; i < MAX_MSGS; i++ )
		current_task->msg_buf[i] = 0;
	for ( i = 0; i < MAX_FILES; i++ )
		current_task->files[i] = 0;

	asm volatile( "sti" );
}

/** \brief The scheduler.
 * Acts along with \ref timer_call to switch tasks.
 */

void switch_task(){
	if ( !current_task )
		return;

	unsigned long esp = 0, ebp = 0, eip = 0, check = 0;
	task_t *temp = (task_t *)current_task;

	eip = read_eip();
	if ( check ){
		return;
	}
	check = 1;

	asm volatile( "mov %%esp, %0" : "=r"(esp));
	asm volatile( "mov %%ebp, %0" : "=r"(ebp));

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

	if ( last_task )
		last_task->time += get_tick() - last_task->last_time;

	current_task->last_time = get_tick();
	current_task->status = S_RUNNING;

	last_task = current_task;

	eip = current_task->eip;
	esp = current_task->esp;
	ebp = current_task->ebp;

	if ( isr_error_count ) isr_error_count--;

	current_dir = current_task->dir;
	set_page_dir( current_dir );
	set_kernel_stack( current_task->stack );
	//set_kernel_stack( current_task->stack );

	//printf( "0x%x\n", current_task->eip );

	asm volatile (" 	\
		cli;		\
		mov %0, %%ecx;	\
		mov %1, %%esp;	\
		mov %2, %%ebp;	\
		mov %3, %%cr3;	\
		sti;\
		jmp *%%ecx" : : "r"(eip), "r"(esp), "r"(ebp), "r"(current_dir->address ));
		//mov $0xdeadbeef, %0;
		//mov $0xdeadbeef, %%eax;
}

int create_process( void (*function)( int, char **, char ** ), char **argv, char **envp, 
			unsigned long start_addr, unsigned long end_addr ){
	asm volatile( "cli" );
	int argc = 0;

	task_t *new_task = (task_t *)kmalloc( sizeof( task_t ), 0, 0 );

	init_task( new_task );
	new_task->eip = (unsigned long)function;
	//kfree((void *) new_task->stack );
	//new_task->stack = 0xb00c0000 + KERNEL_STACK_SIZE;
	//new_task->stack = start_addr + PAGE_SIZE * 2;

	//map_page( 0xbfff0000, 7, current_dir );
	//map_pages( 0xb00c0000, 0xb00f0000, 7, current_dir );
	//flush_tlb( );
	//return 0;

	if ( argv )
		for ( argc = 0; argv[argc]; argc++ );
	else 
		argc = 0;

	/* Copy args for the new process */
	new_task->stack -= sizeof( char * ) * argc + 1;
	char **new_argv = (char **)new_task->stack;
	/*
	char **new_argv = (void *)kmalloc( sizeof( char * ) * argc + 1, 0, 0 );
	*/
	int i;
	for ( i = 0; i < argc; i++ ){
		new_task->stack -= strlen( argv[i] ) + 1;
		new_argv[i] = new_task->stack;
		memcpy( new_argv[i], argv[i], strlen( argv[i] ) + 1 );
		/*
		new_argv[i] = (char *)kmalloc( strlen( argv[i] ) + 1, 0, 0 );
		*/
	}
	new_argv[i] = 0;

	new_task->argv = new_argv;
	new_task->envp = 0;//envp;

	PUSH( new_task->stack, envp );
	PUSH( new_task->stack, new_argv );
	PUSH( new_task->stack, argc );

	new_task->esp = new_task->stack;
	
	add_task( new_task );
	current_task->child_count++;

	asm volatile( "sti" );
	return new_task->id;
}

/** \brief Adds new thread to task queue
 * @param function The address of a void (*)() type function to start the thread at
 * @return The new thread's pid
 */
int create_thread( void (*function)()){
	asm volatile( "cli" );

	task_t *new_task = (task_t *)kmalloc( sizeof( task_t ), 0, 0 );

	init_task( new_task );
	new_task->eip = (unsigned long)function;
	new_task->argv = current_task->argv;
	new_task->envp = current_task->envp;
	add_task( new_task );
	current_task->child_count++;

	asm volatile( "sti" );
	return new_task->id;
}

task_t *init_task( task_t *task ){
	task_t 	*parent = (task_t *)current_task,
		*temp	= (task_t *)task_queue;

	task->esp = task->ebp = task->eip = 0;

	task->id = next_pid++;
	task->next = 0;
	task->parent = parent;
	task->sleep = 0;
	task->time  = 0;
	task->last_time = get_tick();
	task->dir = current_dir;
	task->stack = kmalloc( KERNEL_STACK_SIZE, 1, 0 ) + KERNEL_STACK_SIZE;
	task->esp = task->stack;
	task->ebp = current_task->ebp;

	int i = 0;
	for ( i = 0; i < MAX_MSGS; i++ )
		task->msg_buf[i] = 0;
	for ( i = 0; i < MAX_FILES; i++ )
		task->files[i] = 0;

	task->msg_ptr    = 0;
	task->msg_count  = 0;
	task->file_count = 0;
	task->cwd 	     = parent->cwd;
	task->root 	     = parent->root;

	return task;
}

task_t *add_task( task_t *task ){
	task_t *temp = (task_t *)task_queue;
	while ( temp->next ){
		temp = temp->next;
	}
	temp->next = task;

	return task;
}

void fork_thread( void ){
	//exit_thread();
}

/** \brief Removes calling task from queue */
void exit_thread( ){
	asm volatile( "cli" );
	task_t *temp = (task_t *)current_task;
	task_t *move = (task_t *)task_queue;

	while ( move->next && move->next != temp ) move = move->next;
	move->next = temp->next;

	printf( "parent %d waiting: %d\n", current_task->parent->id, current_task->parent->waiting );
	if ( current_task->parent->waiting ){
		current_task->parent->waiting 	= false;
		current_task->parent->child	= current_task->id;
	}

	if ( current_task->parent->child_count ){
		current_task->parent->child_count--;
	}

	printf( "pid %d exited\n", temp->id );
	switch_task();
	asm volatile( "sti" );
}

/** \brief Removes a pid from task queue
 * @param pid The pid of task to kill
 * @return 1 if the pid doesn't exist, 0 otherwise
 */
int kill_thread( unsigned long pid ){
	asm volatile( "cli" );
	task_t *temp = get_pid_task( pid );
	task_t *move = (task_t *)task_queue;

	if ( !temp ){
		asm volatile( "sti" );
		return 1;
	}
	while ( move->next && move->next != temp )
		move = move->next;

	printf( "[kill] Killing pid %d\n", temp->id );
	move->next = temp->next;

	asm volatile( "sti" );
	return 0;
}

/** \brief Sleep calling task
 * @param time How many scheduler loops to sleep for.
 */
void sleep_thread( unsigned long time ){
	asm volatile( "cli" );

	current_task->sleep = time;
	current_task->status = S_SLEEPING;

	switch_task();
	asm volatile( "sti" );
}

/** \brief Recieve a message from another process
 * @param buf Buffer to read message into
 * @param blocking Whether to block the thread or not
 * @return 0 if a message was recieved, or 1 if there were no messages.
 */
int get_msg( unsigned long blocking, ipc_msg_t *buf ){
	asm volatile( "cli" );
	task_t *temp = (task_t *)current_task;
	unsigned int i = 0;
	int ret = 1;
	asm volatile( "sti" );

	do {
		asm volatile( "cli" );
		if ( temp->msg_count ){
			i = temp->msg_ptr-- % MAX_MSGS;
			memcpy( buf, temp->msg_buf[i], sizeof( ipc_msg_t ));
			temp->msg_count = (temp->msg_count - 1) % MAX_MSGS;
			ret = 0;
			break;
		} else {
			ret = 1;
			temp->status = S_LISTENING;
			switch_task();
		}
		asm volatile( "sti" );
	} while ( blocking && !ret );

	asm volatile( "sti" );
	return ret;
}

/** \brief Send message to another process 
 * @param pid The pid to send to
 * @param msg Pointer to message buffer to send
 * @return 0 if sent, or 1 if the pid couldn't be found
 */
int send_msg( unsigned long pid, ipc_msg_t *msg ){
	asm volatile( "cli" );
	task_t *temp = get_pid_task( pid );
	unsigned int i = 0;

	if ( !temp ){
		asm volatile( "sti" );
		return 1;
	}

	i = ++temp->msg_ptr % MAX_MSGS;
	if ( !temp->msg_buf[i] )
		temp->msg_buf[i] = (void *)kmalloc( sizeof( ipc_msg_t ), 0, 0 );
	memcpy( temp->msg_buf[i], msg, sizeof( ipc_msg_t ));
	temp->msg_count = (temp->msg_count + 1) % MAX_MSGS;

	asm volatile( "sti" );
	return 0;
}

/** \brief Get the \ref task_t structure of a certain pid
 * @param pid The pid to retrieve infomation for.
 * @return Pointer to \ref task_t if found, 0 otherwise
 */
task_t *get_pid_task( unsigned long pid ){
	task_t *temp = (task_t *)task_queue;
	while ( temp ){
		if ( temp->id == pid )
			return temp;
		temp = temp->next;
	}
	return 0;
}
/** \brief Return caller's pid.
 * Is a system call. */
int getpid(){
	return current_task->id;
}

/** \brief Exit current task.
 * Is a system call.
 * @param status Return value of task
 * @return 0.
 */
int exit( char status ){
	current_task->finished = true;
	
	exit_thread();
	return 0;
}

/** \brief Execute a file 
 * @param fd The file descriptor of the file to execute
 * @param argv The arguments for the program
 * @param envp The environment variables for the program
 * @return nothing if successful, -1 otherwise
 */
int fexecve( int fd, char **argv, char **envp ){
	if ( fd >= current_task->file_count || !current_task->files[fd] )
		return -1;

	//int ret = load_elf( fd, argv, envp );
	//load_elf( fd, argv, envp );
	load_elf( fd, argv, envp );
	return 0;

	//return ret; /* If we get here, something went horribly wrong... */
}

int wait( int *status ){
	if ( !current_task->child_count ){
		//printf( "Nothing to wait on\n" );
		*status = 1;
		return -1;
	}
	current_task->waiting = true;

	//printf( "task: %d\n", current_task->waiting );
	while ( current_task->waiting ){
		sleep_thread( 3 );
	}

	current_task->waiting 	= false;
	current_task->child 	= 0;

	*status = 0;

	return current_task->child;
}

/** \brief Dump all running pids to screen */
void dump_pids( void ){
	task_t *temp = (task_t *)task_queue;
	char *buf;
	printf( "pid:\tstate:\t\ttime:\targs:\n" );
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
		printf( "\t%d\t", temp->time );
		if ( temp->argv ){
			int i;
			for ( i = 0; temp->argv[i]; i++ )
				printf( "\"%s\", ", temp->argv[i] );
		}

		printf( "\n" );
		temp = temp->next;
	}
	printf( "\ttotal time: \t%d\n", get_tick());
	temp = (task_t *)task_queue;
}

/** \brief Switch to user mode. (What's on the tin...) */
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
		orl $0x200, %eax;\
		pushl %eax;	\
		pushl $0x1b;	\
		pushl $1f;	\
		iret;		\
		1:		\
	" );
}

/** Switch to user mode, and return to a specific address. */
void switch_to_usermode_jmp( unsigned long addr ){
	set_kernel_stack( current_task->stack + KERNEL_STACK_SIZE );

	asm volatile( "		\
		cli;		\
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
	":: "m"(addr));
}

/*
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

	unsigned long old_esp, old_ebp, offset, new_esp ,new_ebp;
	asm volatile( "mov %%esp, %0" : "=r" (old_esp));
	asm volatile( "mov %%ebp, %0" : "=r" (old_ebp));

	offset = (unsigned long)new_stack_start - initial_esp;
	new_esp = old_esp + offset;
	//new_ebp = old_ebp + offset;
	memcpy((void *)new_esp, (void *)old_esp, initial_esp - old_esp );
}
*/


#endif
