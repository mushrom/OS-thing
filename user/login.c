#include <stdio.h>
#include <syscall.h>
#include <string.h>
#define MAX_LEN 32

int main( int argc, char *argv[] ){
	char *pass = "password";
	char buf[MAX_LEN + 1], temp;

	int i;

	while ( 1 ){
		printf( "password: " );
		read( stdin, &temp, 1 );
		for ( i = 0; i < MAX_LEN && temp != '\n'; i++ ){
			buf[i] = temp;
			read( stdin, &temp, 1 );
		}
		buf[i] = 0;
		
		printf( "Got pass %s\n", buf );
		if ( strcmp( buf, pass ) == 0 ){
			printf( "Password correct\n" );
			int fd = open( "/init/meh", 1 );
			if ( fd < 0 ){
				printf( "Could not open shell\n" );
				continue;
			}

			int ret = fspawn( fd, 0, 0 );
			if ( ret < 0 ){
				printf( "Could not exec shell\n" );
				continue;
			}
			syscall_wait( &ret );
		} else {
			printf( "Incorrect password.\n" );
		}
	}
	return 0;
}
