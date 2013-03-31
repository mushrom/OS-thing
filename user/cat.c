#include <syscall.h>

int main( int argc, char *argv[] ){
	int stdout, fd, i, count;
	char buf[512];

	if ( argc < 2 )
		return 0;

	if (( stdout = syscall_open( "/dev/tty", 2 )) < 0 )
		return 1;

	for ( count = 1; count < argc; count++ ){
		if (( fd = syscall_open( argv[count], 1 )) < 0 )
			return 1;

		do {
			if (( i = syscall_read( fd, &buf, 512 )) < 0 )
				return 1;

			if (( syscall_write( stdout, &buf, i )) < 0 )
				return 1;
		} while ( i );

		syscall_close( fd );
	}

	return 0;
}
