#include <syscall.h>
#include <stdio.h>

int main( int argc, char **argv, char **envp ){
	/*
	syscall_kputs( "argc: " ); print_hex((unsigned long)argc ); 	 syscall_kputs( "\n" );
	syscall_kputs( "args: " ); print_hex((unsigned long)args ); 	 syscall_kputs( "\n" );
	syscall_kputs( "envp: " ); print_hex((unsigned long)envp ); 	 syscall_kputs( "\n" );
	syscall_kputs( "arg1: " ); print_hex((unsigned long)args[0] ); syscall_kputs( "\n" );
	*/
	printf( "argc: 0x%x\n", argc );
	printf( "argv: 0x%x\n", argv );
	printf( "envp: 0x%x\n", envp );
	printf( "argv[0]: 0x%x\n", argv[0] );
	int i;
	for ( i = 0; i < argc; i++ ){
		syscall_kputs( argv[i] );
		syscall_kputs( "\n" );
	}

	return 0;
}
