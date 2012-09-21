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

/*Drivers*/
#include <drivers/driver.h>
#include <sys/console.h>
#include <drivers/kb.h>

#include <sys/kshell.h>
#include <lib/kmacros.h>

uint32_t initial_esp;
extern kernel_driver_t console, drv_tmp;

void test_thing( ){
	int ret = fork();

	//printf( "forkd... %d, %d\n", getpid(), ret );
	printf( "Testing a fork\n" );
	dump_pids();
	while ( 1 ) usleep(100);
}

/* Main kernel code */
void kmain( void* mbd, uint32_t initial_stack, unsigned int magic ){
	#define TIMER_FREQ 50
	initial_esp = initial_stack;
	cls();
	set_color( COLOR_GRAY );

	if ( magic != 0x1BADB002 ){
		kputs( "[\x14-\x17] Multiboot not found...\n" );
	} else {
		kputs( "[\x12+\x17] Multiboot found\n" );
	}

	/* Avoid using any printf calls until after the console driver is set up!
 	 * sys/stdio needs "console" with a good write function, otherwise it'll triple fault */
	init_driver_stuff();	kputs( "[\x12+\x17] initialised driver stuff\n" );
	init_console(); get_driver( USER_OUT ); console = drv_tmp;
	if ( !console.write ){ PANIC( "Could not load console driver" ); }

	init_tables(); 		puts( "[\x12+\x17] initialised tables\n" );
	printf( "    stack at 0x%x\n", initial_stack );

	init_paging(); 		printf( "[\x12+\x17] initialised paging\n" );
	init_keyboard(); 	printf( "[\x12+\x17] initialised keyboard\n" );
	init_tasking(); 	printf( "[\x12+\x17] initialised tasking\n" );
	init_timer(TIMER_FREQ);	printf( "[\x12+\x17] Initialised timer to %uhz\n", TIMER_FREQ );
	init_syscalls(); 	printf( "[\x12+\x17] Initialised syscalls\n" );

	asm volatile ( "sti" );
	console.write( 0, "testing\n", 8 );
	syscall_kputs( "test\n" );

	//test_thing();
	
	kshell( "[\x12shell\x17] $ " );

	//switch_to_user_mode();
	//syscall_kputs( "test\n" );

	while( 1 );
}
