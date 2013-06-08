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
#include <ramfs.h>
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

#include <kconfig.h> 
#include <kshell.h>
#include <kmacros.h>
#include <common.h>
#include <syscall.h>
#include <module.h>
#include <symbols.h>

unsigned int initial_esp; 			/**< esp at start, is set to \ref initial_stack */
unsigned int g_errline = 0;			/**< Error line of last debug, see \ref kmacros.h */
struct multiboot_header *g_mboot_header = 0;

int debug_file = -1;
int stdout_file = -1;

extern unsigned short con_scroll_offset;	/**< How many lines to skip while scrolling */
extern ksymbol_bin_t *ksymbol_table;

extern page_dir_t *kernel_dir;				/**< kernel page directory */
extern page_dir_t *current_dir;

void kinit_prompt( void );

#ifndef NO_DEBUG
/** File of last debug, see \ref kmacros.h */
char *g_errfile = "unknown";
#else
char *g_errfile = "Debugging disabled";
#endif
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
	//kernel_dir = current_dir;
	//printf( "&0x%x: 0x%x->0x%x\n", &kernel_dir, kernel_dir, kernel_dir->address );

	ksymbol_table = init_symbol_bin( 128 ); printf( "[\x12+\x17] initialised kernel symbol table\n" );
	kexport_symbol( "printf", (unsigned long)printf );

	file_node_t *temp = init_vfs();
	fs_mkdir( temp, "dev", 0777 );
	fs_mkdir( temp, "sbin", 0777 );
	//fs_mkdir( temp, "init/bin", 0777 );
	set_fs_root( temp ); 	printf( "[\x12+\x17] initialised vfs\n" );
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

	//debug_file = open( "/dev/ser0", O_WRONLY );

	init_ide( 0x1F0, 0x3F4, 0x170, 0x374, 0x000 );
				printf( "[\x12+\x17] initialised ide\n" );


	printf( "    Kernel init finished: %d ticks\n", get_tick());

	for ( i = 0; i < 80; i++ ) kputs( "=" ); kputs( "\n" );

	con_scroll_offset = 0;

	//create_thread( &main_daemon );
	//cls();
	create_thread( &kinit_prompt );

	while ( 1 ) sleep_thread( 0xffffffff );
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
			fd = open( input, O_EXEC );
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

