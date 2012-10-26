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
#include <fs/initrd.h>

/*
#include <drivers/console.h>
#include <drivers/driver.h>
#include <drivers/kb.h>
#include <drivers/ide.h>
*/
#include <arch/x86/syscall.h>
#include <arch/x86/task.h>
#include <kern/ipc.h>
#include <kern/multiboot.h>

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
	int i = 1;
	while ( 1 ){
		//printf( "%d", 1/0 );
		sleep_thread( 500 );
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
		} else {
			sleep_thread( 3000 );
		}
	}
	exit_thread();
}

/* Main kernel code */
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
		if ( mboot->mods_count ){
			initrd = (void *)*((int *)mboot->mods_addr);
			extern unsigned long placement;
			placement += *(int *)(mboot->mods_addr + 4 );
		} else {
			printf( "    No multiboot modules loaded, can't load initrd\n" );
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
	
	//switch_to_usermode();
	//syscall_kputs( "Woot, it works. ^_^\n" );

	for ( i = 0; i < 3; i++ )
		create_thread( &test );
	for ( i = 0; i < 3; i++ )
		create_thread( &meh );

	create_thread( &kshell );
	create_thread( &main_daemon );
	sleep_thread( 0xffffffff );
	reboot();
}
