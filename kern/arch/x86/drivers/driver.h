#ifndef _kernel_driver_h
#define _kernel_driver_h
#include <lib/stdarg.h>
#include <lib/stdint.h>
#include <lib/stdio.h>
#include <fs/fs.h>
#define MAX_NAME 16
#define MAX_DRIVERS 8

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

//void load_driver( file_type_t );
void init_driver_stuff( void );
void dump_drivers( void );
//int unload_driver( file_node_t );

//kernel_driver_t gen_driver( init_func, write_func, read_func, pwrite_func, pread_func, ioctl_func );
#endif
