/* =====================================================*\
 * Mah kernel, v0x00000001
 * Simple machine
 * Props to wiki.osdev.org. ^_^
\*======================================================*/

#include <sys/skio.h>
#include <arch/arch.h>
#include <lib/stdint.h>
#include <lib/stdio.h>
#include <sys/syscall.h>
#include <mem/paging.h>

#include <fs/fs.h>
#include <fs/devfs.h>

/*Drivers*/
#include <drivers/console.h>
#include <drivers/driver.h>
#include <drivers/kb.h>
#include <drivers/ide.h>

#include <sys/kshell.h>
#include <lib/kmacros.h>
#include <mem/alloc.h>

uint32_t initial_esp;
#ifndef NO_DEBUG
char *g_errfile = "unknown";
#else
char *g_errfile = "Debugging disabled";
#endif
unsigned int g_errline = 0;
page_dir_t *kernel_dir;

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
	#define TIMER_FREQ 50
	initial_esp = initial_stack;
	cls();
	set_color( COLOR_LGRAY );

	kputs( "[\x19obsidian\x17 OS]\n" );
	for ( i = 0; i < 80; i++ ) kputs( "=" ); kputs( "\n" );

	if ( magic != 0x2BADB002 ){
		kputs( "[\x14-\x17] Multiboot not found...\n" );
	} else {
		kputs( "[\x12+\x17] Multiboot found\n" );
	}

	init_tables(); 		kputs( "[\x12+\x17] initialised tables\n" );
	asm volatile ( "sti" );
	init_timer(TIMER_FREQ);	printf( "[\x12+\x17] Initialised timer to %uhz\n", TIMER_FREQ );
	init_paging(); 		printf( "[\x12+\x17] initialised paging\n" );
	//init_heap( KHEAP_START, 0x10000, kernel_dir );	printf( "[\x12+\x17] initialised heap\n" );
	init_vfs();		printf( "[\x12+\x17] initialised vfs\n" );
	init_devfs();		printf( "[\x12+\x17] initialised + mounted devfs\n" );

	init_driver_stuff();	kputs( "[\x12+\x17] initialised driver stuff\n" );
	init_console();
	init_keyboard(); 	//printf( "[\x12+\x17] initialised keyboard\n" );
	init_ide( 0x1F0, 0x3F4, 0x170, 0x374, 0x000 );
				printf( "[\x12+\x17] initialised ide\n" );
	init_syscalls(); 	printf( "[\x12+\x17] Initialised syscalls\n" );
	init_shell();
	//init_tasking(); 	printf( "[\x12+\x17] initialised tasking\n" );
	for ( i = 0; i < 80; i++ ) kputs( "=" ); kputs( "\n" );

	asm volatile ( "sti" );

	char *str = "[\x12+\x17] Syscalls + paging operational\n";
	memcpy((void*)0xc0001000, str, strlen(str));
	syscall_kputs((char *)0xc0001000 );

	//test_thing();
	//char *args[] = { "meh", "adev", 0 };
	
	//sh_list( 1, 0 );
	kshell( 1, 0 );

	//switch_to_user_mode();
	//syscall_kputs( "test\n" );

	while( 1 );
}
