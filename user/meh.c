#include "syscall.h"

void thread( void ){
	syscall_kputs( "Threading works. ^_^\n" );
	syscall_exit( 0 );
}

int main( void ){
	char *str = "User-space program, calling in.\n";
	int fd = syscall_open( "/dev/tty", 0 );
	int i;
	while ( *str++ ) i++;
	syscall_write( fd, "User-space program, calling in.\n", i );
	syscall_close( fd );
	syscall_thread( &thread );
	return 0;
}
