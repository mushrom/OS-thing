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
	memset((void *)task_queue, 0, sizeof( task_t ));

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

	current_task->msg_buf = 0;
	current_task->files = (void *)kmalloc( sizeof( struct file_decript * ) * MAX_FILES, 0, 0 );
	memset( current_task->files, 0, sizeof( struct file_descript * ) * MAX_FILES );

	memset((void *)current_task->sighandle, 0, sizeof( ksignal_h ) * MAX_SIGNALS );

	set_kernel_stack( current_task->stack );

	asm volatile( "sti" );
}

/** \brief The scheduler.
 * Acts along with \ref timer_call to switch tasks.
 */

void switch_task(){
	if ( !current_task )
		return;

	asm volatile( "cli" );
	unsigned long esp = 0, ebp = 0, eip = 0, eax = 0, check = 0;
	task_t *temp = (task_t *)current_task;

	eip = read_eip();
	if ( check ){
		asm volatile( "sti" );
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

	if ( current_task->sig_queued && !current_task->in_sig && current_task->sighandle[ current_task->signal ]){
		current_task->sig_queued = false;
		current_task->in_sig = true;
		current_task->old_eip = current_task->eip;
		//asm volatile( "mov %%eax, %0" : "=r"(eax));
		//current_task->old_eax = eax;
		jmp_to_signal( current_task->signal, current_task->sighandle[ current_task->signal ]);
		current_task->signal = 0;
	}

	eip = current_task->eip;
	esp = current_task->esp;
	ebp = current_task->ebp;

	current_dir = current_task->dir;
	set_page_dir( current_dir );

	/*
	*(unsigned *)(esp - 512) = 0;
	if ( esp < 0xc0000000 && esp > 0x200000 ){
		printf( "esp: 0x%x ebp: 0x%x  current_dir: 0x%x\n", esp, ebp, current_dir->address );
		temp->sleep = 0xffff;
		//asm volatile( "hlt" );
		//return;
	}
	*/

	if ( isr_error_count ) isr_error_count--;

	//set_kernel_stack( current_task->stack );
	//set_kernel_stack( current_task->stack );

	//printf( "0x%x\n", current_task->eip );

	//return;
	asm volatile (" 	\
		cli;		\
		mov %0, %%ecx;	\
		mov %1, %%esp;	\
		mov %2, %%ebp;	\
		sti;\
		jmp *%%ecx" : : "r"(eip), "r"(esp), "r"(ebp));
		//mov $0xdeadbeef, %0;
		//mov $0xdeadbeef, %%eax;
		//*mov %3, %%cr3;	
}

// Work in progress, documentation to come
int create_process( void (*function)( int, char **, char ** ), char **argv, char **envp, 
			unsigned long start_addr, unsigned long end_addr ){
	asm volatile( "cli" );
	int argc = 0;

	task_t *new_task = (task_t *)kmalloc( sizeof( task_t ), 0, 0 );
	memset( new_task, 0, sizeof( task_t ));

	//printf( "task start: 0x%x, task end: 0x%x\n", start_addr, end_addr );

	init_task( new_task );
	new_task->eip = (unsigned long)function;
	//new_task->start_addr = start_addr;
	//new_task->end_addr = end_addr;
	new_task->maps = memmap_create( start_addr, end_addr, MEM_READ | MEM_EXEC | MEM_USER, 1 );
	new_task->brk = (char *)end_addr;

	if ( argv )
		for ( argc = 0; argv[argc]; argc++ );
	else 
		argc = 0;

	/* Copy args for the new process */
	new_task->stack -= sizeof( char * ) * argc + 1;
	char **new_argv = (char **)new_task->stack;
	int i;
	for ( i = 0; i < argc; i++ ){
		new_task->stack -= strlen( argv[i] ) + 1;
		new_argv[i] = (char *)new_task->stack;
		memcpy( new_argv[i], argv[i], strlen( argv[i] ) + 1 );
	}
	new_argv[i] = 0;

	new_task->argv = new_argv;
	new_task->envp = 0;//envp;

	PUSH( new_task->stack, envp );
	PUSH( new_task->stack, new_argv );
	PUSH( new_task->stack, argc );

	new_task->esp = new_task->stack;
	new_task->tid = 0;
	
	add_task( new_task );
	current_task->child_count++;

	asm volatile( "sti" );
	//return 0;
	//return new_task->id;
}

/** \brief Adds new thread to task queue
 * @param function The address of a void (*)() type function to start the thread at
 * @return The new thread's pid
 */
int create_thread( void (*function)()){
	asm volatile( "cli" );

	task_t *new_task = (task_t *)kmalloc( sizeof( task_t ), 0, 0 );
	memset( new_task, 0, sizeof( task_t ));

	init_task( new_task );
	new_task->maps = current_task->maps;
	new_task->eip = (unsigned long)function;
	new_task->argv = current_task->argv;
	new_task->envp = current_task->envp;
	add_task( new_task );
	current_task->child_count++;
	new_task->tid = current_task->tid++;

	asm volatile( "sti" );
	return new_task->id;
}

task_t *init_task( task_t *task ){
	task_t 	*parent = (task_t *)current_task;

	task->esp = task->ebp = task->eip = 0;

	task->id = next_pid++;
	task->next = 0;
	task->parent = parent;
	task->sleep = 0;
	task->time  = 0;
	task->last_time = get_tick();
	task->dir = current_dir;
	task->stack = kmalloc( KERNEL_STACK_SIZE, 1, 0 ) + KERNEL_STACK_SIZE;
	memset((void *)( task->stack - KERNEL_STACK_SIZE ), 0, KERNEL_STACK_SIZE );
	task->esp = task->stack;
	task->ebp = current_task->ebp;

	task->msg_buf = 0;
	task->tid = 0;
	task->files = (void *)kmalloc( sizeof( struct file_decript * ) * MAX_FILES, 0, 0 );
	memset( task->files, 0, sizeof( struct file_descript * ) * MAX_FILES );

	memset((void *)task->sighandle, 0, sizeof( ksignal_h ) * MAX_SIGNALS );

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

void remove_task( task_t *task ){
	asm volatile( "cli" );
	task_t *temp = (task_t *)task;
	task_t *move = (task_t *)task_queue;

	while ( move->next && move->next != temp ) move = move->next;
	move->next = temp->next;

	//printf( "parent %d waiting: %d\n", task->parent->id, task->parent->waiting );
	if ( task->parent->waiting ){
		task->parent->waiting 	= false;
		task->parent->child	= task->id;
	}

	if ( task->parent->child_count ){
		task->parent->child_count--;
	}

	/*
	if ( task->start_addr && task->end_addr && task->tid == 0 ){
		free_pages( task->start_addr, task->end_addr, task->dir );
	}
	*/
	asm volatile( "sti" );
}

/** \brief Removes calling task from queue */
void exit_thread( ){
	asm volatile( "cli" );
	remove_task( current_task );
	switch_task( );
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
	remove_task( temp );

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

void *sbrk( int inc ){
	memmap_t *map = memmaps_check( current_task->maps, current_task->brk );
	/*
	char *ret = current_task->brk;
	current_task->brk += inc;
	*/
	char *ret = current_task->brk;
	printf( "[kern] got map 0x%x\n", map );
	current_task->brk += inc;
	map->end += inc;

	return (void *)ret;
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
		} else {
			printf( "(none) " );
		}

		printf( "\n" );
		temp = temp->next;
	}
	printf( "\ttotal time: \t%d\n", get_tick());
	temp = (task_t *)task_queue;
}

/** \brief Switch to user mode. (What's on the tin...) */
void switch_to_usermode( void ){
	set_kernel_stack( current_task->stack );

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
	//set_kernel_stack( current_task->stack );

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
