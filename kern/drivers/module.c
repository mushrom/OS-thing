#ifndef _kernel_module_c
#define _kernel_module_c
#include <module.h>

int load_module( char *path, int flags ){
	int fd, ret;

	fd = open( path, 0 );
	if ( !fd ){
		printf( "Could not find path\n" );
		return -1;
	}
	
	ret = fexecve( fd, 0, 0 );

	printf( "Could not exec file\n" );

	return ret;
}
	
#endif
