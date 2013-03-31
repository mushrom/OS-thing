#include <stdio.h>
#include <syscall.h>

int main( int argc, char *argv[] ){
	if ( argc < 2 )
		return 1;

	int fd, i = 0;
	char buf;
	if (( fd = open( argv[1], 1 )) < 0 ){
		printf( "Could not open file \"%s\"\n", argv[1] );
		return 1;
	}

	while (( read( fd, &buf, 1 )) > 0 ){
		printf( "0x%x ", buf );
		if ( i++ % 8 == 0 )
			printf( "\n" );
	}

	return 0;
}
