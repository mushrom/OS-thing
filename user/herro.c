#include <syscall.h>
#include <stdio.h>

int main( int argc, char *argv[], char *envp[] ){
	printf( "Hello, from pid %d. ^_^\n", getpid());

	return 0;
}
