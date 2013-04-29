#include <syscall.h>

int main( int argc, char *argv[] ){
	int stdout = 1, fd, i, count;
	char buf[512];

	if ( argc < 2 )
		return 0;

	/*
	if (( stdout = open( "/dev/tty", 2 )) < 0 )
		return 1;
	*/

	for ( count = 1; count < argc; count++ ){
		if (( fd = open( argv[count], 1 )) < 0 )
			return 1;

		do {
			if (( i = read( fd, &buf, 512 )) < 0 )
				return 1;

			if (( write( stdout, &buf, i )) < 0 )
				return 1;
		} while ( i );

		close( fd );
	}

	return 0;
}
