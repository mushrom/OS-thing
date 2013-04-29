#ifndef _kernel_task_h
#define _kernel_task_h

#include <paging.h>
#include <alloc.h>
#include <kmacros.h>
#include <ipc.h>
#include <fs.h>
#include <elf.h>
#include <common.h>

//#define KERNEL_STACK_SIZE 4096
#define KERNEL_STACK_SIZE 0x1000
#define MAX_MSGS 32
#define MAX_FILES 32
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
			uid,
			gid;

	unsigned long addr_start;
	unsigned long addr_end;

	unsigned long stack;
	unsigned long sleep;
	unsigned long status;
	unsigned long time;
	unsigned long last_time;

	bool finished;
	bool waiting;
	char ret;

	struct page_dir *dir;
	struct task *next;
	struct task *prev;
	struct task *parent;

	unsigned long child;
	unsigned long child_count;
	int child_status;

	ipc_msg_t *msg_buf[MAX_MSGS];
	unsigned char msg_ptr;
	unsigned long msg_count;
	char **argv;
	char **envp;

	struct file_node     *root;
	struct file_node     *cwd;
	struct file_descript *files[MAX_FILES];
	unsigned long file_count;
	unsigned long file_highest;
} task_t;

void init_tasking( );
int  create_process( void (*)(int, char **, char **), char **, char ** );
int  create_thread( void (*)());
int  kill_thread( unsigned long pid );
void sleep_thread( unsigned long time );
void exit_thread( );

void gen_msg( ipc_msg_t *msg, unsigned char type, unsigned long size, void *buf );
int  send_msg( unsigned long pid, ipc_msg_t *msg );
int  get_msg( unsigned long blocking, ipc_msg_t *buf );

void dump_pids( void );
void switch_to_usermode( void );
void switch_to_usermode_jmp( unsigned long addr );
void switch_task();
void move_stack( void *, unsigned long size );

int getpid( void );
int exit( char status );
int fexecve( int fd, char **argv, char **envp );
int load_flat_bin( int fd );
int wait( int * );

task_t *get_pid_task( unsigned long pid );
task_t *init_task( task_t *task );
task_t *add_task(  task_t *task );

#endif
