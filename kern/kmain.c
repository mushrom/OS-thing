/*! \page kernel Kernel
 *
 * The kernel enters at \ref kmain.
 *
 * \ref kmain initialises the kernel, initialises the essentials, 
 * launches a \ref switch_to_usermode "user-mode" thread, then 
 * sleeps.
 * 
 * The \ref syscall.c "system call" interface uses interrupt 0x50 (decimal 80) to enter.
 *
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
#include <syscall.h>

unsigned int initial_esp; 			/**< esp at start, is set to \ref initial_stack */
unsigned int g_errline = 0;			/**< Error line of last debug, see \ref kmacros.h */
extern unsigned short con_scroll_offset;	/**< How many lines to skip while scrolling */
page_dir_t *kernel_dir;				/**< kernel page directory */

#ifndef NO_DEBUG
/** File of last debug, see \ref kmacros.h */
char *g_errfile = "unknown";
#else
char *g_errfile = "Debugging disabled";
#endif

/** The process that does the jump to usermode */
void user_daemon( ){
	switch_to_usermode();
	char *str = "[\x12+\x17] Usermode is operational.\n";
	//int fp = syscall_open( "/dev/tty", 0);
	int fp = syscall_open( "/init/daemon", 0);
	/*
	syscall_write( fp, str, 32 );
	*/
	syscall_fexecve( fp, 0, 0 );
	syscall_close( fp );
	syscall_exit(0);
}

/** A test daemon that listens for a message, and executes commands
 * based on the message
 */
void main_daemon( ){
	ipc_msg_t msg;
	int ret, meh;
	while ( 1 ){
		ret = syscall_get_msg( MSG_BLOCK, &msg );
		if ( ret == 0 ){
			meh = 0xfffffff;
			switch ( msg.msg_type ){
				case MSG_EXIT:
					reboot();
					break;
				case MSG_STATUS:
					printf( "Main daemon, how can I help you?\n" );
					while( meh-- );
					break;
			}
		}
	}
}

/** A process to test sleeping/switching tasks */
void test( ){
	while ( 1 ){
		sleep_thread( 1000 );
	}
	exit_thread();
}

/** A process to test the ipc's messaging */
void meh( ){
	ipc_msg_t msg, reply;
	int ret;
	while ( 1 ){
		ret = get_msg( MSG_NO_BLOCK, &msg );
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

			if ( msg.msg_type == 234 ){
				int fd = open( "/init/blarg", 0 );
				if ( fd > -1 ){
					printf( "Testing...(%d)\n", fd );
					fexecve( fd, 0, 0 );
				}
				close( fd );
				printf( "Complete\n" );
			}
		} else {
			//sleep_thread( 1 );
		}
	}
	exit_thread();
}

/** \brief Main kernel code 
 * @param mboot multiboot structure provided by bootloader
 * @param initial_stack initial esp pointer, pushed by loader.s
 * @param magic multiboot magic value
 * */
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

	init_tables(); 		printf( "[\x12+\x17] initialised tables\n" );
	init_timer(TIMER_FREQ);	printf( "[\x12+\x17] Initialised timer to %uhz\n", TIMER_FREQ );
	asm volatile ( "sti" );
	init_paging(); 		printf( "[\x12+\x17] initialised paging\n" );
	init_vfs();		printf( "[\x12+\x17] initialised vfs\n" );
	init_devfs();		printf( "[\x12+\x17] initialised + mounted devfs\n" );
	if ( initrd ){
		init_initrd( initrd ); 	
		printf( "[\x12+\x17] initialised initrd\n" );
	}

	init_keyboard(); 	//printf( "[\x12+\x17] initialised keyboard\n" );
	init_console();
	init_ide( 0x1F0, 0x3F4, 0x170, 0x374, 0x000 );
				printf( "[\x12+\x17] initialised ide\n" );
	init_syscalls(); 	syscall_kputs( "[\x12+\x17] Initialised syscalls\n" );
	init_tasking(); 	printf( "[\x12+\x17] initialised tasking\n" );
	init_shell();
	for ( i = 0; i < 80; i++ ) kputs( "=" ); kputs( "\n" );
	con_scroll_offset = 0;
	//sleep_thread( 0xffffffff );

	//for ( i = 0; i < 100; i++ )
	create_thread( &test );
	for ( i = 0; i < 3; i++ )
		create_thread( &meh );

	create_thread( &main_daemon );
	create_thread( &user_daemon );
	create_thread( &kshell );
	//switch_to_usermode_jmp((unsigned long)&user_daemon );
	sleep_thread( 0xffffffff );
}
