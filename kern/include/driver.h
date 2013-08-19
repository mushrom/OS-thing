#ifndef _kernel_driver_h
#define _kernel_driver_h
#include <stdarg.h>
#include <stdint.h>
#include <stdio.h>
#include <fs.h>

typedef struct driver {
	char *name;
}

//void load_driver( file_type_t );
void init_driver_stuff( void );
void dump_drivers( void );
//int unload_driver( file_node_t );

//kernel_driver_t gen_driver( init_func, write_func, read_func, pwrite_func, pread_func, ioctl_func );
#endif
