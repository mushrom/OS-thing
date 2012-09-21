#ifndef _kernel_driver_h
#define _kernel_driver_h
#include <lib/stdarg.h>
#include <lib/stdint.h>
#include <lib/stdio.h>
#define MAX_NAME 16

/* Drivers [ 09/21/12 ]
 * At the moment this is a bit messy, but in the future it should make it
 * easier to have modules to be loaded in the kernel while running.
 * 
 * The drivers are small structs with function pointers, that are copied into a 
 * dynamic array of kernel_driver_t using "register_driver"
 *
 * To get a driver for whatever purposes, the get_driver function returns the 
 * first driver with an appropriate driver_type_t into drv_tmp. 
 * This will be improved in the future.
 */

typedef enum {
	USER_IN   = 1,
	USER_OUT  = 2,
	DISK	  = 4,
	FILE_S    = 8
} driver_type_t;

typedef int (*init_func)(int);
typedef int (*write_func)(int, void *, size_t );
typedef int (*read_func)(int, void *, size_t );
typedef int (*pwrite_func)(int, void *, size_t, unsigned long );
typedef int (*pread_func)(int, void *, size_t, unsigned long );
typedef int (*ioctl_func)(int, unsigned long, ... );

typedef struct kernel_driver {
	uint32_t	id;
	uint32_t	type;
	init_func   	init;

	write_func  	write;
	read_func  	read;
	pwrite_func 	pwrite;
	pread_func  	pread;
	ioctl_func  	ioctl;
} kernel_driver_t;

int register_driver( kernel_driver_t );
void get_driver( driver_type_t );
void init_driver_stuff( void );
void dump_drivers( void );

//kernel_driver_t gen_driver( init_func, write_func, read_func, pwrite_func, pread_func, ioctl_func );
#endif
