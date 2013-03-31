#include "syscall.h"
#include "stdio.h"
#define MAX_LEN 256

int main( void ){
	char cmd[MAX_LEN];
	char *args[MAX_LEN];
	char buf;

	int running = 1, j, i = 0, arg_no, fd;
	int ret;

	while ( running ){
		printf( "[\x15user\x17] $ " );
		read(  stdin, &buf, 1 );
		for ( i = 0; i < MAX_LEN; i++ ) cmd[i] = 0;
		for ( i = 0; buf != '\n' && i < MAX_LEN; ){
			if ( buf == '\b' ){
				cmd[i] = 0;
				if ( i > 0 ){
					i--;
					write( stdout, "\b", 1 );
				}
			} else {
				cmd[i++] = buf;
				write( stdout, (char *)&buf, 1 );
			}
			read( stdin, &buf, 1 );
		}
		cmd[i] = 0;
		write( stdout, "\n", 1 );

		args[0] = &cmd[0];
		for ( arg_no = j = 0, i = 1; cmd[j]; j++ ){
			if ( cmd[j] == ' ' ){
				args[i++] = &cmd[j] + 1;
				cmd[j] = 0; 
				arg_no++;
			}
		}
		args[i] = 0;
		
		if ( strlen( args[0] )){
			for ( j = 0; j < arg_no + 1; j++ ){
				write( stdout, args[j], strlen( args[j] ));
				write( stdout, "\n", 1 );
			}
			fd = open( args[0], 0 );
			if ( fd < 0 ){
				printf( "Could not open file\n" );
				continue;
			} else {
				fspawn( fd, 0, 0 );
				syscall_wait( &ret );
			}
		}
	}	
	return 0;
}
