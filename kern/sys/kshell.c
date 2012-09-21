#ifndef _kernel_shell_c
#define _kernel_shell_c
#include <sys/kshell.h>

#define CMD_LIMIT 128 

void kshell( char *PS1 ){
	char buf, cmd[ CMD_LIMIT ], *str;
	int running = 1, i = 0, j = 0;
	
	while ( running ){
		printf( "%s", PS1 );
		/* get input */
		pause();
		for ( i = 0; ( buf = get_in_char( )) != '\n' && i < CMD_LIMIT; ){
			if ( buf == '\b' ){
				cmd[i] = 0;
				if ( i > 0 ){
					i--;
					kputchar( buf );
				}
			} else { 
				cmd[i++] = buf;
				kputchar( buf );
			}
			pause();
		}
		kputchar( '\n' );
		cmd[i] = 0;

		/* Find command */
		if ( strcmp( cmd, "exit" ) == 0 ){
			printf( "Exitting shell...\n" );
			running = 0;
		} else if ( strcmp( cmd, "test" ) == 0 ){
			printf( "Testing shell functionality\n" );
		} else if ( strcmp( cmd, "sleep" ) == 0 ){
			for ( j = 3; j > 0; j-- ){
				printf( "sleeping, resuming in %d seconds..\r", j );
				wait( 1 );
			}
			kputchar( '\n' );
		} else if ( strcmp( cmd, "assert" ) == 0 ){
			assert( 1 < 0, );
		} else if ( strcmp( cmd, "fork" ) == 0 ){
			int ret = fork();
			printf( "Forked successfully to pid %u\n", ret );
		} else if ( strcmp( cmd, "ps" ) == 0 ){
			dump_pids();
		} else if ( strcmp( cmd, "switch-task" ) == 0 ){
			printf( "switching from pid %d\n", getpid());
			switch_task();
			while ( 1 ) usleep( 50 );
		} else if ( strcmp( cmd, "getpid" ) == 0 ){
			printf( "%d\n", getpid());
		} else if ( strcmp( cmd, "clear" ) == 0 ){
			cls();
		} else if ( strcmp( cmd, "usermode" ) == 0 ){
			switch_to_user_mode();
			syscall_kputs( "usermode test\n" );
		} else if ( strcmp( cmd, "alloc-test" ) == 0 ){
			uint32_t addr;
			for ( j = 3; j--; ){
				str = kmalloc( strlen( cmd ) + 1, 0, &addr );
				memcpy( str, cmd, strlen( cmd ) + 1);
				printf( "\"%s\" at 0x%x (real 0x%x)\n", str, str, addr );
				//kfree( str );
			}
		} else if ( strcmp( cmd, "dump-regs" ) == 0 ){
			asm volatile( "int $48" );
		} else if ( strcmp( cmd, "int-test" ) == 0 ){
			printf( "Testing interrupts...\n" );
			asm volatile("int $0x03");
		} else if ( strcmp( cmd, "drivers" ) == 0 ){
			dump_drivers();
		} else if ( strcmp( cmd, "pagefault" ) == 0 ){
			printf( "Faulting...\n" );
			printf((char *) 0xa0000000 );
		} else if ( strcmp( cmd, "panic" ) == 0 ){
			PANIC( "Shell-invoked panic" );
		} else if ( strlen( cmd ) > 0 ){
			printf( "[debug] \"%s\", %d\n", cmd, i );
		}
	}
}
#endif
