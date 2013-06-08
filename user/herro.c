#include <syscall.h>
#include <stdio.h>
#include <alloc.h>
#include <signal.h>

int sighandle( int s ){
	printf( "Caught signal, like a baws: %d\n", s );
	
	sigreturn( 0 );
	return 0;
}

int main( int argc, char *argv[], char *envp[] ){
	printf( "Hello, from pid %d. ^_^\n", getpid());
	char *ptr = malloc( 0x1001 );
	printf( "sbrk got 0x%x\n", ptr );
	memset( ptr, 0, 0x1001 );
	printf( "Woop, got here. ^_^\n" );
	free( ptr );
	printf( "ptr was free'd.\n" );

	printf( "Testing signal at 0x%x... ", sighandle );
	signal( SIGINT, sighandle );
	kill( getpid( ), SIGINT );

	printf( "Done, I'm out.\n" );
	return 0;
}
