#ifndef _kernel_shell_c
#define _kernel_shell_c
#include <sys/kshell.h>

#define STR_LIMIT 128 
#define CMD_LIMIT 64

extern file_node_t *fs_root;
file_node_t *fs_cwd;
int cmd_count = 0;
shell_cmd_t commands[ CMD_LIMIT ];

int  sh_test( int argc, char **argv );
int  sh_help( int argc, char **argv );
int sh_clear( int argc, char **argv );
int  sh_list( int argc, char **argv );
int  sh_dump( int argc, char **argv );
int sh_write( int argc, char **argv );
int  sh_read( int argc, char **argv );
int    sh_cd( int argc, char **argv );
int  sh_atoi( int argc, char **argv );
int sh_sleep( int argc, char **argv );
int sh_debug( int argc, char **argv );

void init_shell( ){
	register_shell_func( "ls", sh_list );
	register_shell_func( "cd", sh_cd );
	register_shell_func( "write", sh_write );
	register_shell_func( "read", sh_read );
	register_shell_func( "clear", sh_clear );
	register_shell_func( "debug", sh_debug );
	register_shell_func( "dump", sh_dump );
	register_shell_func( "help", sh_help );
	register_shell_func( "test", sh_test );
	register_shell_func( "atoi", sh_atoi );
	register_shell_func( "sleep", sh_sleep );
	register_shell_func( "shell", kshell );
}

void register_shell_func( char *name, shell_func_t function ){
	commands[ cmd_count ].name = name;
	commands[ cmd_count ].function = function;
	cmd_count++;
}

int kshell( int argc, char **argv ){
	char buf, *cmd, **args, *PS1 = "[\x12shell\x17] $ ";
	fs_cwd = fs_root;
	int running = 1, i = 0, j = 0, arg_no = 0, cmd_found = 0;

	if ( argc > 1 )
		PS1 = argv[1];

	cmd  = (void *)kmalloc( STR_LIMIT, 0, 0 );
	args = (void *)kmalloc( STR_LIMIT, 0, 0 );
	args[0] = cmd;
	
	while ( running ){
		printf( "%s", PS1 );

		/* Get input */
		pause();
		cmd_found = 0;
		for ( i = 0; ( buf = get_in_char( )) != '\n' && i < STR_LIMIT; ){
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

		/* Split input up */
		arg_no = 1;
		for ( j = 1; *cmd != 0; cmd++ ){
			if ( *cmd == ' ' ){
				args[j++] = cmd + 1;
				*cmd = 0;
				arg_no++;
			}
			if ( *cmd == '\'' || *cmd == '\"' ){
				*cmd = 0; args[j - 1]++;
				for ( cmd++; *cmd != '\'' && *cmd != '\"' && *cmd != 0; cmd++ );
				*cmd = 0;
			}
		}
		cmd = args[0];

		/* Find command */
		for ( j = 0; j < cmd_count; j++ ){
			if ( commands[j].name && commands[j].function && 
				strcmp( args[0], commands[j].name ) == 0 ){
				commands[j].function( arg_no, args );
				cmd_found = 1;
			}
		}
		if ( !cmd_found && strlen( args[0] )){
			printf( "Unknown command: \"%s\"\n", args[0] );
		}
	}

	return 0;
}

int sh_test( int argc, char **argv ){
	int i = 0;
	for ( i = 0; i < argc; i++ ){
		printf( "arg %d: %s\n", i, argv[i] );
	}
	return 0;
}

int sh_help( int argc, char **argv ){
	int i;
	printf( "commands availible:\n" );
	for ( i = 0; i < cmd_count; i++ ){
		printf( "\t%s", commands[i].name );
		if ((i+1)%5 == 0 && i) printf( "\n" );
	}
	printf( "\n" );
	return 0;
}

int sh_clear( int argc, char **argv ){
	cls();
	return 0;
}

int  sh_list( int argc, char **argv ){
	file_node_t *fp = fs_cwd, *file_thing;
	struct dirp *dir;
	struct dirent *entry;
	struct vfs_stat sb;
	int items = 0;
	char color = 0x17;

	if ( argc > 1 ){
		fp = fs_find_node( fs_cwd, argv[1] );
		if ( !fp ){
			printf( "Could not find file\n" );
			return -1;
		}
	}

	dir = fs_opendir( fp );
	if ( dir ){
		while (( entry = fs_readdir( dir ))){
			file_thing = fs_find_node( fp, (char *)entry->name );
			fs_stat( file_thing, &sb );

			if ( sb.type == FS_DIR )
				color = 0x19;
			if ( sb.type == FS_CHAR_D )
				color = 0x15;
			if ( sb.type == FS_BLOCK_D )
				color = 0x14;

			printf( "%c%s\x17\t", color, entry->name );
			items++;
			if ( items % 8 == 0 )
				printf( "\n" );
			color = 0x17;
		}
		printf( "\n" );
		fs_closedir( fp );
		//printf( "%d items\n", items );
	} else {
		printf( "Could not open directory\n" );
	}

	return 0;
}

int sh_write( int argc, char **argv ){
	file_node_t *fp = fs_cwd;
	int bytes_written = 0;

	if ( argc < 3 ){
		printf( "Need arguments!\n" );
		return 0;
	}

	fs_open( fp, argv[1], 0777 );
	fp = fs_find_node( fp, argv[1] );
	if ( fp ){
		bytes_written = fs_write( fp, argv[2], strlen( argv[2] ) + 1);
	} else {
		bytes_written = -1;
	}
	if ( bytes_written == -1 ){
		printf( "Could not write to file\n" );
	} else {
		printf( "Wrote %d bytes\n", bytes_written );
	}

	return 0;
}

int  sh_read( int argc, char **argv ){
	file_node_t *fp = fs_cwd;
	char buf[0x2000];
	int size = 512;
	int bytes_read = 0;
	int i = 0;

	if ( argc < 2 ){
		printf( "Need arguments!\n" );
		return 0;
	}
	if ( argc > 2 )
		size = atoi( argv[2] );

	//fs_open( fp, argv[1], 0777 );
	fp = fs_find_node( fp, argv[1] );
	if ( fp ){
		bytes_read = fs_read( fp, buf, size );
	} else {
		printf( "Could not find file\n" );
		return 0;
	}
	if ( bytes_read == -1 ){
		printf( "Could not read file\n" );
	} else {
		printf( "read %d bytes\n", bytes_read );
		if ( bytes_read ){
			for ( i = 0; i < bytes_read; i++ )
				printf( "%c", buf[i] );
			printf( "\n" );
		}
	}
	memset( buf, 0, 0x2000 );
	
	return 0;
}

int  sh_dump( int argc, char **argv ){
	asm volatile( "int $0x30" );
	return 0;
}

int    sh_cd( int argc, char **argv ){
	file_node_t *fp;
	if ( argc > 1 ){
		fp = fs_find_node( fs_cwd, argv[1] );
		if ( fp ){
			fs_cwd = fp;
		} else {
			printf( "Could not cd\n" );
		}
	} else {
		fs_cwd = fs_root;
	}
	return 0;
}

int  sh_atoi( int argc, char **argv ){
	if ( argc < 2 )
		return 0;
	printf( "%d\n", atoi( argv[1] ));

	return 0;
}

int sh_sleep( int argc, char **argv ){
	if ( argc < 2 )
		return 0;
	int i = atoi( argv[1] );

	while ( i-- ){
		printf( "Resuming in %d seconds..\r", i + 1 );
		wait( 1 );
	}
	printf( "\n" );

	return i;
}

int sh_debug( int argc, char **argv ){
	printf( "%s:%d\n", g_errfile, g_errline );
	return 0;
}

#endif
