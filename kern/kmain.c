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
#include <null_device.h>
#include <abc_device.h>

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
	file_node_t nodebuf, mountbuf, *fp;

	con_scroll_offset = 2;
	kputs( "[\x19obsidian\x17 OS]\n" );
	for ( i = 0; i < 80; i++ ) kputs( "=" ); kputs( "\n" );

	if ( magic != 0x2BADB002 ){
		PANIC( "[\x14-\x17] Multiboot not found..." );
	} else {
		printf( "[\x12+\x17] Multiboot found, header at 0x%x\n", mboot );
		if ( mboot->flags & MULTIBOOT_FLAG_MEM ){
			printf( "mem low: %ukb, mem high: %ukb\n", mboot->mem_lower, mboot->mem_upper );
		} else { 
			PANIC( "Multiboot header has no mem map" );
		}
		if ( mboot->flags & MULTIBOOT_FLAG_CMDLINE ){
			printf( "cmd = %s\n", mboot->cmdline );
		}
		if ( mboot->flags & MULTIBOOT_FLAG_MODS && mboot->mods_count ){
			initrd = (void *)*((int *)mboot->mods_addr);
			initrd_size = *(int *)(mboot->mods_addr + 4 ) - (unsigned int)initrd;
			printf( "initrd size: %d bytes\n", initrd_size );

			extern unsigned long placement;
			placement = *(int *)(mboot->mods_addr + 4 );
		} else {
			printf( "No multiboot modules loaded\n" );
		}
		g_mboot_header = mboot;
	}

	init_tables(); 			
	init_timer(TIMER_FREQ);		
	asm volatile ( "sti" );
	init_paging( mboot ); 		

	ksymbol_table = init_symbol_bin( 128 ); 
	kexport_symbol( "printf", (unsigned long)printf );

	file_system_t *temp = init_ramfs();
	register_fs( temp, 0 );

	set_fs_root( temp ); 	

	printf( "Got here\n" );
	printf( "[ ] Set virtual file system root\n" );

	init_syscalls(); 	
	init_tasking(); 

	//init_ide( 0x1F0, 0x3F4, 0x170, 0x374, 0x000 );
	i = mkdir( "/dev", 0700 );
	printf( "mkdir: 0x%x\n", i );
	
	if ( initrd ){
		//initrd_create( initrd ); 	
	}

	i = fs_find_path( "/dev", 1, &nodebuf );
	printf( "[ ] Device setup... 0x%x\n", i );
	if ( i < 0 ){
		printf( "error: %d: ", -i );
		if ( i == -ENOENT )
			printf( "couldn't find file" );
		else if ( i == -ENOTDIR )
			printf( "apparently it's not a directory" );
		else
			printf( "dunno cap'n" );

		printf( "\n" );
	}

	//i = fs_find_path( "/dev/kb0", 1, &nodebuf );

	//fs_mount( &nodebuf, fp, 0, 0 );
	//kfree( fp );
	
	register_fs( keyboard_create( ), 0 );

	printf( "[ ] Dumping filesystems...\n" );
	extern llist_node_t *file_system_list;
	llist_node_t *blarg;
	for ( blarg = file_system_list; blarg->next; blarg = blarg->next ){
		temp = blarg->data;
		printf( "%d:\t\"%s\"\n", temp->id, temp->name );
		if ( temp->id == 321 ){
			printf( "Got here\n" );

			mountbuf.fs = temp;
			mountbuf.inode = temp->i_root;
			mknod( "/dev/kb0", 0777, FS_CHAR_D );
			printf( "Searching...\n" );
			fs_find_path( "dev/kb0", 1, &nodebuf );

			fs_mount( &nodebuf, &mountbuf, 0, 0 );
			fs_find_path( "/dev/kb0", 1, &nodebuf );
		}
	}
	
	/*
	fs_mknod( temp, "kb0", 0777, FS_CHAR_D );
	fs_mknod( temp, "null", 0777, FS_CHAR_D );
	fs_mknod( temp, "ser0", 0777, FS_CHAR_D );
	fs_mknod( temp, "tty", 0777, FS_CHAR_D );

	temp = fs_find_path( "/dev/kb0", 1 );
	fs_mount( keyboard_create( ), temp, 0, 0 );
	
	temp = fs_find_path( "/dev/tty", 1 );
	fs_mount( console_create( ), temp, 0, 0 );

	temp = fs_find_path( "/dev/null", 1 );
	fs_mount( null_create( ), temp, 0, 0 );

	temp = fs_find_path( "/dev/ser0", 1 );
	fs_mount( serial_create( ), temp, 0, 0 );
	*/

	/*
	fs_mount( temp, 
	*/

	/*
	char *long_idestr = "/dev/ide0";
	char *idestr = long_idestr + 5;
	file_node_t *buf;
	int j = 0;
	for ( j = 0; j < 4; j++, idestr[3]++ ){
		buf = ide_create( j );
		if ( buf ){
			temp = fs_find_path( "/dev", 1 );
			fs_mknod( temp, idestr, 0777, FS_BLOCK_D );
			temp = fs_find_path( long_idestr, 1 );
			fs_mount( buf, temp, 0, 0 );
		}
	}
	*/

	/*
	init_serial();
	init_keyboard(); 	
	init_console();

	init_ide( 0x1F0, 0x3F4, 0x170, 0x374, 0x000 );
	*/

	//ext2fs_dump_info( "/dev/ide1" );

	printf( "    Kernel init finished: %d ticks\n", get_tick());
	for ( i = 0; i < 80; i++ ) kputs( "=" ); kputs( "\n" );

	con_scroll_offset = 0;

	create_thread( &kinit_prompt );

	while( 1 ) asm ("hlt");
	//while ( 1 ) sleep_thread( 0xffffffff );
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

	//mkdir( "/user", 0777 );
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
				fd = syscall_open( args[i] + l + 1, O_EXEC );
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

