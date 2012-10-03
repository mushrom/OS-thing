#ifndef _kernel_driver_c
#define _kernel_driver_c
#include <drivers/driver.h>

//kernel_driver_t *driver_list;
file_type_t	*dev_root,
		*dev_node;

void load_driver( file_type_t driver_node ){
	//unsigned int i;
}

/*
kernel_driver_t gen_driver( 	driver_type_t type, init_func init, write_func write, read_func read, 
				pwrite_func pwrite, pread_func pread, ioctl_func ioctl ){
	kernel_driver_t buf;

	buf.type   = type;
	buf.init   = init;
	buf.write  = write;
	buf.read   = read;
	buf.pwrite = pwrite;
	buf.pread  = pread;
	buf.ioctl  = ioctl;

	return buf;
}*/

void init_driver_stuff( void ){
	//driver_list = kmalloc( sizeof( kernel_driver_t ) * MAX_DRIVERS, 1, 0 );
	/*

	drv_dummy.type	= 0;
	memcpy( drv_dummy.name, "dummy", 5 );
	drv_dummy.id	= 0x0;
	drv_dummy.init 	= 0;
	drv_dummy.write = 0;
	drv_dummy.read 	= 0;
	drv_dummy.pwrite = 0;
	drv_dummy.pread = 0;
	drv_dummy.ioctl = 0;
	drv_dummy.unload = 0;
	*/
}

void dump_drivers( void ){
	/*
	unsigned int i;
	for ( i = 0; i < driver_p; i++ ){
		printf( "driver %d: name=\"%s\", id=0x%x, type=", i, driver_list[i].name, driver_list[i].id );
		if ( driver_list[i].type & USER_IN )  printf("Input (user), ");
		if ( driver_list[i].type & USER_OUT ) printf("Output (user), ");
		if ( driver_list[i].type & DISK )     printf("Disk, ");
		if ( driver_list[i].type & FILE_S )   printf("File system, ");
		printf( "\n" );
	}
	*/
}

int unload_driver( file_type_t driver_node ){
	return 0;
}
	
#endif
