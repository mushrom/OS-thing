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
#include <fat.h>
#include <serial.h>

#include <kconfig.h> #include <kshell.h>
#include <kmacros.h>
#include <common.h>
#include <syscall.h>
#include <module.h>

unsigned int initial_esp; 			/**< esp at start, is set to \ref initial_stack */
unsigned int g_errline = 0;			/**< Error line of last debug, see \ref kmacros.h */
extern unsigned short con_scroll_offset;	/**< How many lines to skip while scrolling */
page_dir_t *kernel_dir;				/**< kernel page directory */
struct multiboot_header *g_mboot_header = 0;

int debug_file = -1;
int stdout_file = -1;

#ifndef NO_DEBUG
/** File of last debug, see \ref kmacros.h */
char *g_errfile = "unknown";
#else
char *g_errfile = "Debugging disabled";
#endif

/** A test daemon that listens for a message, and executes commands
 * based on the message
 */
void main_daemon( ){
	ipc_msg_t msg;
	int ret, meh;
	while ( 1 ){
		ret = get_msg( MSG_BLOCK, &msg );
		if ( ret == 0 ){
			meh = 0xfffff;
			switch ( msg.msg_type ){
				case MSG_EXIT:
					reboot();
					break;
				case MSG_STATUS:
					printf( "Main daemon, how can I help you?\n" );
					while( meh-- );
					break;
			}
			printf( "meh\n" );
		}
	}
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

/** \brief Initialises userland. */
void kinit_prompt( void ){
	char input[256];
	char *initstr = "init";
	char *args[16];
	char *cmd;
	char buf= 0;
	int fd 	= 0, 
	    j 	= 0,
	    l 	= 0,
	    i 	= 0;

	mkdir( "/user", 0777 );
	//mkdir( "/user/dev", 0777 );
	//mount( "/dev", "/user/dev", 0, 0 );

	/* Check to see if the init is specified in multiboot command line... */
	if ( g_mboot_header && g_mboot_header->flags & 2 ){
		cmd = (void *)g_mboot_header->cmdline;
		args[0] = (char *)&input;
		/* split multiboot cmd line up */
		for ( j = 1, i = 0; cmd[i] && i < 256 && j < 16; i++ ){
			input[i] = cmd[i];
			if ( input[i] == ' ' ){
				args[j++] = &input[i] + 1;
				input[i] = 0;
			}
		}
		input[i] = 0;
		args[j] = 0;
		for ( l = i = 0; i < j; i++ ){
			for ( l = 0; args[i][l] && args[i][l] != '='; l++ ){
				if ( args[i][l] != initstr[l] ){
					l = 0;
					break;
				}
			}
			if ( l == 4 ){
				fd = syscall_open( args[i] + l + 1, 0 );
				if ( fd < 0 ){
					printf( "Could not open init file\n" );
					break;
				}

				switch_to_usermode();
				syscall_fexecve( fd, 0, 0 );
				syscall_exit( 0 );
			}
		}
	}
	memset( input, 0, 256 );

	/* Nope, have the user manually enter it */
	printf( "Enter path of an executable, or \"!\" for built-in shell\n" );
	while ( 1 ) {
		printf( "init > " );
		pause();
		for ( i = 0; ( buf = get_in_char()) != '\n' && i < 256; ){
			if ( buf == '\b' ){
				input[i] = 0;
				if ( i > 0 ){
					i--;
					printf( "\b" );
				}
			} else {
				input[i++] = buf;
				printf( "%c", buf );
			}
			pause();
		}
		input[i] = 0;
		printf( "\n" );
		if ( !i )
			continue;
		if ( input[0] == '!' ){
			init_shell();
			kshell();
		} else {
			fd = open( input, 0 );
			if ( fd < 0 ){
				fd = 0;
				printf( "Could not open file \"%s\".\n", input );
			} else {
				/* chroot( "/user" ); */
				switch_to_usermode();
				syscall_fexecve( fd, 0, 0 );
				syscall_exit(0);
			}
		}
		memset( input, 0, 256 );
	}	
	exit_thread();
}

/** \brief Main kernel code 
 * @param mboot multiboot structure provided by bootloader
 * @param initial_stack initial esp pointer, pushed by loader.s
 * @param magic multiboot magic value
 * */
void kmain( struct multiboot_header *mboot, uint32_t initial_stack, unsigned int magic ){
	int i, initrd_size;
	initrd_header_t *initrd = 0;
	initial_esp = initial_stack;
	cls();
	set_color( COLOR_LGRAY );

	con_scroll_offset = 2;
	kputs( "[\x19obsidian\x17 OS]\n" );
	for ( i = 0; i < 80; i++ ) kputs( "=" ); kputs( "\n" );

	if ( magic != 0x2BADB002 ){
		PANIC( "[\x14-\x17] Multiboot not found..." );
	} else {
		printf( "[\x12+\x17] Multiboot found, header at 0x%x\n", mboot );
		if ( mboot->flags & MULTIBOOT_FLAG_MEM ){
			printf( "    mem low: %ukb, mem high: %ukb\n", mboot->mem_lower, mboot->mem_upper );
		} else { 
			PANIC( "Multiboot header has no mem map" );
		}
		if ( mboot->flags & MULTIBOOT_FLAG_CMDLINE ){
			printf( "    cmd = %s\n", mboot->cmdline );
		}
		if ( mboot->mods_count && mboot->flags & MULTIBOOT_FLAG_MODS ){
			initrd = (void *)*((int *)mboot->mods_addr);
			initrd_size = *(int *)(mboot->mods_addr + 4 ) - (unsigned int)initrd;
			printf( "    initrd size: %d bytes\n", initrd_size );

			extern unsigned long placement;
			//placement = (*(int *)(mboot->mods_addr + 4 ) & 0xfffff000) + 0x1000;
			placement = *(int *)(mboot->mods_addr + 4 );
			//placement += *(int *)(mboot->mods_addr + 4 );
			printf( "    new placement: 0x%x\n", placement );
		} else {
			printf( "    No multiboot modules loaded\n" );
		}
		g_mboot_header = mboot;
	}

	init_tables(); 		kputs( "[\x12+\x17] initialised tables\n" );
	init_timer(TIMER_FREQ);	kputs( "[\x12+\x17] Initialised timer\n" );
	asm volatile ( "sti" );
	init_paging( mboot ); 		printf( "[\x12+\x17] initialised paging\n" );

	init_vfs();		printf( "[\x12+\x17] initialised vfs\n" );
	init_devfs();		printf( "[\x12+\x17] initialised + mounted devfs\n" );
	if ( initrd ){
		init_initrd( initrd ); 	
		printf( "[\x12+\x17] initialised initrd\n" );
	}

	init_syscalls(); 	kputs( "[\x12+\x17] Initialised syscalls\n" );
	init_tasking(); 	printf( "[\x12+\x17] initialised tasking\n" );

	init_serial();
	init_keyboard(); 	printf( "[+] initialised keyboard\n" );
	init_console();

	init_ide( 0x1F0, 0x3F4, 0x170, 0x374, 0x000 );
				printf( "[\x12+\x17] initialised ide\n" );

	//test_fatfs( "/dev/ide0", 63 * 512 );
	printf( "    Kernel init finished: %d ticks\n", get_tick());

	for ( i = 0; i < 80; i++ ) kputs( "=" ); kputs( "\n" );

	con_scroll_offset = 0;

	//create_thread( &main_daemon );
	create_thread( &kinit_prompt );
	sleep_thread( 0xffffffff );
}
