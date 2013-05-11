#include <syscall.h>
int module_init( void ){
	void (*printf)( char *fmt, ... ) = syscall_kget_symbol( "printf" );

	printf( "woop ^_^\n" );
	
	//char *str = "test\n";
	//int fd = open( "/dev/tty", O_WRONLY );
	//while( 1 );
	//kputs( "Test module\n" );
	return 0;
}

int module_exit( void ){
	return 0;
}
