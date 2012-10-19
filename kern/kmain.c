/* =====================================================*\
 * Mah kernel, v0x00000001
 * Simple machine
 * Props to wiki.osdev.org. ^_^
\*======================================================*/

#include <arch/arch.h>
#include <sys/skio.h>
#include <sys/syscall.h>
#include <lib/stdint.h>
#include <lib/stdio.h>
#include <mem/paging.h>
#include <mem/alloc.h>

#include <fs/fs.h>
#include <fs/devfs.h>

#include <drivers/console.h>
#include <drivers/driver.h>
#include <drivers/kb.h>
#include <drivers/ide.h>

#include <sys/kshell.h>
#include <lib/kmacros.h>

uint32_t initial_esp;
unsigned int g_errline = 0;
extern unsigned short con_scroll_offset;
page_dir_t *kernel_dir;

#ifndef NO_DEBUG
char *g_errfile = "unknown";
#else
char *g_errfile = "Debugging disabled";
#endif

void test_thing( ){
	//int ret = fork();

	//printf( "forkd... %d, %d\n", getpid(), ret );
	printf( "Testing a fork\n" );
	//dump_pids();
	while ( 1 ) usleep(100);
}

/* Main kernel code */
void kmain( void* mbd, uint32_t initial_stack, unsigned int magic ){
	int i;
	#define TIMER_FREQ 1000
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
	init_timer(TIMER_FREQ);	printf( "[\x12+\x17] Initialised timer to %uhz\n", TIMER_FREQ );
	/* kmalloc init'd along with paging, check mem/paging.c */
	init_paging(); 		printf( "[\x12+\x17] initialised paging\n" );
	init_vfs();		printf( "[\x12+\x17] initialised vfs\n" );
	init_devfs();		printf( "[\x12+\x17] initialised + mounted devfs\n" );

	asm volatile( "sti" );
	init_driver_stuff();	kputs( "[\x12+\x17] initialised driver stuff\n" );
	init_console();
	init_keyboard(); 	//printf( "[\x12+\x17] initialised keyboard\n" );
	init_ide( 0x1F0, 0x3F4, 0x170, 0x374, 0x000 );
				printf( "[\x12+\x17] initialised ide\n" );
	init_syscalls(); 	syscall_kputs( "[\x12+\x17] Initialised syscalls\n" );
	init_shell();
	//init_tasking(); 	printf( "[\x12+\x17] initialised tasking\n" );
	for ( i = 0; i < 80; i++ ) kputs( "=" ); kputs( "\n" );
	con_scroll_offset = 0;

	//asm volatile ( "sti" );

	//test_thing();
	//char *args[] = { "meh", "adev", 0 };
	
	//sh_list( 1, 0 );
	kshell( 1, 0 );

	//switch_to_user_mode();

	while( 1 );
}
