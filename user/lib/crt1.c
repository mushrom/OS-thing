#include <syscall.h>
#include <fs.h>
extern int main( int, char **, char ** );

int crt1_main( int argc, char **argv, char **envp ){
	int in  = syscall_open( "/dev/kb0", O_RDONLY );
	int out = syscall_open( "/dev/tty", O_WRONLY );
	int err = syscall_open( "/dev/tty", O_WRONLY );

	return main( argc, argv, envp );
}
