/* =====================================================*\
 * Mah kernel, v0x00000001
 * Simple machine
 * Props to wiki.osdev.org. ^_^
\*======================================================*/

#include <sys/skio.h>
#include <lib/itoa.h>
#include <arch/arch.h>
#include <lib/stdint.h>
#include <lib/stdio.h>
#include <sys/syscall.h>

/*Drivers*/
#include <sys/console.h>
#include <sys/kb.h>

#include <sys/kshell.h>
#include <lib/kmacros.h>

uint32_t initial_esp;

void test_thing( ){
	int ret = fork();

	while ( 1 ){
		printf( "forkd... %d, %d\n", getpid(), ret );
		usleep( 5000 );
	}
}

/* Main kernel code */
void kmain( void* mbd, uint32_t initial_stack, unsigned int magic ){
	#define TIMER_FREQ 50
	initial_esp = initial_stack;
	cls();
	set_color( COLOR_GRAY );

	if ( magic != 0x1BADB002 ){
		printf( "[\x14-\x17] Multiboot not found... (got 0x%x)\n", magic );
	} else {
		kputs( "[\x12+\x17] Multiboot found\n" );
	}
	printf( "    stack at 0x%x\n", initial_stack );

	//arch_init();
	init_tables();
	kputs( "[\x12+\x17] initialised tables\n" );

	register_interrupt_handler( 0x30, &dump_registers );

	init_paging();
	kputs( "[\x12+\x17] initialised paging\n" );

	init_keyboard();
	printf( "[\x12+\x17] initialised keyboard\n" );

	init_tasking();
	printf( "[\x12+\x17] initialised tasking\n" );

	init_timer( TIMER_FREQ );
	printf( "[\x12+\x17] Initialised timer to %uhz\n", TIMER_FREQ );

	init_syscalls();
	printf( "[\x12+\x17] Initialised syscalls\n" );

	asm volatile ( "sti" );

	//test_thing();
	//int ret = fork();
	kshell( "[\x12shell\x17] $ " );

	switch_to_user_mode();

	/*
	kshell( "[\x12shell\x17] $ " );
	kputs( "test\n" );
	while( 1 );
	*/
	//kputs( "Returned...\n" );
	//while ( 1 ){ 
		//printf( "waiting %u\n", 2 );
		//kputs( "meh\n" );
//		wait( 2 );
//	}
	/*
	while ( 1 ){
		printf( "meh, %d, %d\n", getpid( ), ret );
		usleep( 20000 );
	}
	kputs( "bleh\n" );

	printe( "Currently running processes: \n" );
	dump_pids();
	//while( 1 ) usleep( 1 );
	for ( i = 0; i < 10; i++ ){
		printf( "uptime: %uh:%um:%us\r", i / 3600, i / 60, i % 60 );
		wait( 1 );
	}*/
	while ( 1 ) usleep(1);

	//printf( 0xa0000000 );
	
	//kfree( testalloc );
}
