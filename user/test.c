#include <syscall.h>

void waste( void ){
	while( 1 );
}

int main( void ){
	int i;
	for ( i = 0; i < 100; i++ )
		syscall_thread( &waste );
}
