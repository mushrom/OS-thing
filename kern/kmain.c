/*! \page kernel Kernel
 *
 * The kernel enters at \ref kmain.
 *
 * \ref kmain initialises the kernel, initialises the essentials, 
 * launches a \ref switch_to_usermode "user-mode" thread, then 
 * sleeps.
 * 
 * The \ref syscall.c "system call" interface uses interrupt 0x50 (decimal 80) to enter.
 */

#include <skio.h>
#include <stdint.h>
#include <stdio.h>
#include <paging.h>
#include <alloc.h>

#include <fs.h>
#include <devfs.h>
#include <initrd.h>

/*
#include <drivers/console.h>
#include <drivers/driver.h>
#include <drivers/kb.h>
#include <drivers/ide.h>
*/
#include <syscall.h>
#include <task.h>
#include <ipc.h>
#include <multiboot.h>
#include <timer.h>
#include <init_tables.h>
#include <kb.h>
#include <ide.h>

#include <kshell.h>
#include <kmacros.h>
#include <common.h>

unsigned int initial_esp;
unsigned int g_errline = 0;
extern unsigned short con_scroll_offset;
page_dir_t *kernel_dir;

#ifndef NO_DEBUG
char *g_errfile = "unknown";
#else
char *g_errfile = "Debugging disabled";
#endif

void user_daemon( ){
	switch_to_usermode();
	syscall_kputs( "[\x12+\x17] Usermode is operational\n" );
	//exit_thread();
	while( 1 );
}

void main_daemon( ){
	ipc_msg_t msg;
	int ret;
	while ( 1 ){
		ret = get_msg( &msg, MSG_BLOCK );
		if ( ret == 0 ){
			switch ( msg.msg_type ){
				case MSG_EXIT:
					reboot();
					break;
				case MSG_STATUS:
					printf( "Main daemon, how can I help you?\n" );
					break;
			}
		}
	}
}

void test( ){
	while ( 1 ){
		//printf( "pid %d, calling in\n", getpid( ));
		sleep_thread( 1000 );
		switch_task();
	}
	exit_thread();
}

void meh( ){
	ipc_msg_t msg, reply;
	int ret;
	while ( 1 ){
		//kputchar( 'b' );
		//printf( "pid \x18%d\x17 sleeping\n", getpid());
		//sleep_thread( 10 );
		ret = get_msg( &msg, MSG_NO_BLOCK );
		if ( ret == 0 ){
			printf( "pid %d: got message 0x%x (%s) from pid %d\n",
				 getpid(), msg.msg_type, msg_lookup( msg.msg_type ), msg.sender );

			if ( msg.msg_type == MSG_STATUS ){
				reply.msg_type 	= MSG_ACK;
				reply.sender	= getpid();
				send_msg( msg.sender, &reply );
			}

			if ( msg.msg_type == 123 )
				exit_thread();

			if ( msg.msg_type == 234 )
				ret = ret/0;
		} else {
			sleep_thread( 3000 );
		}
	}
	exit_thread();
}

/*! \brief Main kernel code */
void kmain( struct multiboot_header *mboot, uint32_t initial_stack, unsigned int magic ){
	int i;
	initrd_header_t *initrd = 0;
	initial_esp = initial_stack;
	cls();
	set_color( COLOR_LGRAY );

	con_scroll_offset = 2;
	kputs( "[\x19obsidian\x17 OS]\n" );
	for ( i = 0; i < 80; i++ ) kputs( "=" ); kputs( "\n" );

	if ( magic != 0x2BADB002 ){
		kputs( "[\x14-\x17] Multiboot not found...\n" );
	} else {
		printf( "[\x12+\x17] Multiboot found, header at 0x%x\n", mboot );
		if ( mboot->flags & 2 )
			printf( "    cmd = %s\n", mboot->cmdline );
		if ( mboot->mods_count ){
			initrd = (void *)*((int *)mboot->mods_addr);
			extern unsigned long placement;
			placement += *(int *)(mboot->mods_addr + 4 );
		} else {
			printf( "    No multiboot modules loaded\n" );
		}
	}

	init_tables(); 		kputs( "[\x12+\x17] initialised tables\n" );
	init_paging(); 		printf( "[\x12+\x17] initialised paging\n" );
	init_timer(TIMER_FREQ);	printf( "[\x12+\x17] Initialised timer to %uhz\n", TIMER_FREQ );
	asm volatile ( "sti" );
	init_vfs();		printf( "[\x12+\x17] initialised vfs\n" );
	init_devfs();		printf( "[\x12+\x17] initialised + mounted devfs\n" );
	if ( initrd ){
		init_initrd( initrd ); 	
		printf( "[\x12+\x17] initialised initrd\n" );
	}

	init_console();
	init_keyboard(); 	//printf( "[\x12+\x17] initialised keyboard\n" );
	init_ide( 0x1F0, 0x3F4, 0x170, 0x374, 0x000 );
				printf( "[\x12+\x17] initialised ide\n" );
	init_syscalls(); 	syscall_kputs( "[\x12+\x17] Initialised syscalls\n" );
	init_tasking(); 	printf( "[\x12+\x17] initialised tasking\n" );
	init_shell();
	for ( i = 0; i < 80; i++ ) kputs( "=" ); kputs( "\n" );
	con_scroll_offset = 0;
	
	//syscall_kputs( "Woot, it works. ^_^\n" );

	//for ( i = 0; i < 3; i++ )
		create_thread( &test );
	for ( i = 0; i < 3; i++ )
		create_thread( &meh );

	create_thread( &main_daemon );
	create_thread( &user_daemon );
	create_thread( &kshell );
	sleep_thread( 0xffffffff );
}