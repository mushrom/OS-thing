#include "syscall.h"
#define MAX_LEN 256
int strlen( char *str ){
	int i;
	for ( i = 0; str[i]; i++ );

	return i;
}

int main( void ){
	char cmd[MAX_LEN];
	char *args[MAX_LEN];
	char *PS1 = "[\x15user\x17] $ ";
	char *no_open = "Could not open file\n";
	char buf;

	int running = 1, j, i = 0, stdin, stdout, arg_no, fd;

	stdin  = syscall_open( "/dev/kb0", 0 );
	stdout = syscall_open( "/dev/tty", 0 );

	if ( stdin < 0 || stdout < 0 )
		return 1;

	while ( running ){
		syscall_write( stdout, PS1, strlen( PS1 ));
		syscall_read(  stdin, &buf, 1 );
		for ( i = 0; i < MAX_LEN; i++ ) cmd[i] = 0;
		for ( i = 0; buf != '\n' && i < MAX_LEN; ){
			if ( buf == '\b' ){
				cmd[i] = 0;
				if ( i > 0 ){
					i--;
					syscall_write( stdout, "\b", 1 );
				}
			} else {
				cmd[i++] = buf;
				syscall_write( stdout, (char *)&buf, 1 );
			}
			syscall_read( stdin, &buf, 1 );
		}
		cmd[i] = 0;
		syscall_write( stdout, "\n", 1 );

		args[0] = &cmd[0];
		for ( arg_no = j = 0, i = 1; cmd[j]; j++ ){
			if ( cmd[j] == ' ' ){
				args[i++] = &cmd[j] + 1;
				cmd[j] = 0; 
				arg_no++;
			}
		}

		for ( j = 0; j < arg_no + 1; j++ ){
			syscall_write( stdout, args[j], strlen( args[j] ));
			syscall_write( stdout, "\n", 1 );
		}
		
		fd = syscall_open( args[0], 0 );
		if ( fd < 0 ){
			syscall_write( stdout, no_open, strlen( no_open ));
			continue;
		} else {
			syscall_fexecve( fd, 0, 0 );
		}
	}	
	return 0;
}
