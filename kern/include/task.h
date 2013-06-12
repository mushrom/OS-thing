#ifndef _kernel_task_h
#define _kernel_task_h

#include <paging.h>
#include <alloc.h>
#include <kmacros.h>
#include <signal.h>
#include <ipc.h>
#include <fs.h>
#include <elf.h>
#include <common.h>
#include <memmap.h>

//#define KERNEL_STACK_SIZE 4096
#define KERNEL_STACK_SIZE 0x800
#define MAX_MSGS 32
#define MAX_FILES 32
#define MAX_SIGNALS 32
#define PUSH( stack, data ) { stack -= sizeof( data ); memcpy((void *)stack, &data, sizeof( data )); }

enum {
	S_RUNNING,
	S_SLEEPING,
	S_LISTENING,
	S_SENDING
};

//struct page_dir;
//struct file_descript;

typedef struct task {
	unsigned long 	eip,
			esp,
			ebp,
			id,
			tid,
			uid,
			gid;

	/*
	unsigned long start_addr;
	unsigned long end_addr;
	*/
	struct memmap *maps;
	unsigned long stack;
	char *brk;
	char **argv;
	char **envp;

	char ret;

	unsigned long sleep;
	unsigned long status;
	unsigned long time;
	unsigned long last_time;
	bool finished;
	bool waiting;

	struct page_dir *dir;
	struct task *next;
	struct task *prev;
	struct task *parent;

	unsigned long child;
	unsigned long child_count;
	int child_status;

	struct ipc_msg *msg_buf;
	unsigned char msg_ptr;
	unsigned long msg_count;

	ksignal_h sighandle[ MAX_SIGNALS ];
	bool sig_queued;
	bool in_sig;
	signal_t signal;
	unsigned long 	old_eip;

	struct file_node     *root;
	struct file_node     *cwd;
	struct file_descript **files;
	unsigned long file_count;
	unsigned long file_highest;
} task_t;

void init_tasking( );
int  create_process( void (*)(int, char **, char **), char **, char **, unsigned long, unsigned long );
int  create_thread( void (*)());
int  kill_thread( unsigned long pid );
void sleep_thread( unsigned long time );
void exit_thread( );

void dump_pids( void );
void switch_to_usermode( void );
void switch_to_usermode_jmp( unsigned long addr );
void switch_task();
//void move_stack( void *, unsigned long size );

int getpid( void );
int exit( char status );
int fexecve( int fd, char **argv, char **envp );
int load_flat_bin( int fd );
int wait( int * );
void *sbrk( int );

task_t *get_pid_task( unsigned long pid );
task_t *init_task( task_t *task );
task_t *add_task(  task_t *task );
void remove_task(  task_t *task );

#endif
