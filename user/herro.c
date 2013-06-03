#include <syscall.h>
#include <stdio.h>
#include <alloc.h>

int main( int argc, char *argv[], char *envp[] ){
	printf( "Hello, from pid %d. ^_^\n", getpid());
	char *ptr = malloc( 0x1001 );
	printf( "sbrk got 0x%x\n", ptr );
	memset( ptr, 0, 0x1001 );
	printf( "Woop, got here. ^_^\n" );
	free( ptr );
	printf( "ptr was free'd.\n" );

	return 0;
}
