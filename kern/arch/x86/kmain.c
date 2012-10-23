/*======================================================*\
 * Mah kernel, v0x00000001
 * Simple machine
 * Props to wiki.osdev.org. ^_^
\*======================================================*/

#include <arch/arch.h>
#include <sys/skio.h>
#include <lib/stdint.h>
#include <lib/stdio.h>
#include <mem/paging.h>
#include <mem/alloc.h>

#include <fs/fs.h>
#include <fs/devfs.h>

/*
#include <drivers/console.h>
#include <drivers/driver.h>
#include <drivers/kb.h>
#include <drivers/ide.h>
*/
#include <arch/arch.h>
#include <arch/x86/syscall.h>
#include <arch/x86/task.h>
#include <kern/ipc.h>

#include <sys/kshell.h>
#include <lib/kmacros.h>

unsigned int initial_esp;
unsigned int g_errline = 0;
extern unsigned short con_scroll_offset;
page_dir_t *kernel_dir;

#ifndef NO_DEBUG
char *g_errfile = "unknown";
#else
char *g_errfile = "Debugging disabled";
#endif

void test( ){
	int i = 1;
	while ( 1 ){
		sleep_thread( 500 );
		switch_task();
	}
	exit_thread();
}

void meh( ){
	ipc_msg_t msg;
	while ( 1 ){
		//kputchar( 'b' );
		//printf( "pid \x18%d\x17 sleeping\n", getpid());
		//sleep_thread( 10 );
		get_msg( &msg );
		printf( "pid %d: got message 0x%x from pid %d, woot\n", getpid(), msg.msg_type, msg.sender );
		if ( msg.msg_type == 123 )
			exit_thread();
	}
	exit_thread();
}

/* Main kernel code */
void kmain( uint32_t initial_stack, unsigned int magic ){
	int i;
	initial_esp = initial_stack;
	cls();
	set_color( COLOR_LGRAY );

	con_scroll_offset = 2;
	kputs( "[\x19obsidian\x17 OS]\n" );
	for ( i = 0; i < 80; i++ ) kputs( "=" ); kputs( "\n" );

	if ( magic != 0x2BADB002 ){
		kputs( "[\x14-\x17] Multiboot not found...\n" );
	} else {
		kputs( "[\x12+\x17] Multiboot found\n" );
	}

	init_tables(); 		kputs( "[\x12+\x17] initialised tables\n" );
	init_paging(); 		printf( "[\x12+\x17] initialised paging\n" );
	init_timer(TIMER_FREQ);	printf( "[\x12+\x17] Initialised timer to %uhz\n", TIMER_FREQ );
	asm volatile ( "sti" );
	init_tasking(); 	printf( "[\x12+\x17] initialised tasking\n" );
	init_vfs();		printf( "[\x12+\x17] initialised vfs\n" );
	init_devfs();		printf( "[\x12+\x17] initialised + mounted devfs\n" );

	init_console();
	init_keyboard(); 	//printf( "[\x12+\x17] initialised keyboard\n" );
	init_ide( 0x1F0, 0x3F4, 0x170, 0x374, 0x000 );
				printf( "[\x12+\x17] initialised ide\n" );
	init_syscalls(); 	syscall_kputs( "[\x12+\x17] Initialised syscalls\n" );
	init_shell();
	for ( i = 0; i < 80; i++ ) kputs( "=" ); kputs( "\n" );
	con_scroll_offset = 0;
	
	//switch_to_usermode();
	//syscall_kputs( "Woot, it works. ^_^\n" );

	for ( i = 0; i < 3; i++ )
		create_thread( &test );
	for ( i = 0; i < 3; i++ )
		create_thread( &meh );
	kshell( 1, 0 );
	reboot();
}
