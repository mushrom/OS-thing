#ifndef _kernel_driver_c
#define _kernel_driver_c
#include <drivers/driver.h>

kernel_driver_t *driver_list;
kernel_driver_t drv_dummy, 
		console, 
		drv_input,
		drv_disk,
		drv_tmp;
unsigned int driver_p = 0;

#define MAX_DRIVERS 8

int register_driver( kernel_driver_t new_driver ){
	if ( driver_p < MAX_DRIVERS ){
		driver_list[ driver_p ] = new_driver;
		//printf( "Registered driver %d, %u\n", driver_p, new_driver.id );

		if ( new_driver.init ){
			new_driver.init( 0 );
		}
		return driver_p++;
	} else {
		return -1;
	}
}

void get_driver( driver_type_t type ){
	unsigned int i;
	//drv_tmp = drv_dummy;

	for ( i = 0; i < driver_p; i++ ){
		if ( driver_list[i].type & type ){
			//printf( "Driver write func: 0x%x\n", driver_list[i].write );
			//drv_tmp = driver_list[i];
			drv_tmp = driver_list[i];
		}
	}
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
	driver_list = kmalloc( sizeof( kernel_driver_t ) * MAX_DRIVERS, 1, 0 );

	/*
	drv_dummy.type	= 0;
	drv_dummy.id	= 123;
	drv_dummy.init 	= 0;
	drv_dummy.write = 0;
	drv_dummy.read 	= 0;
	drv_dummy.pwrite = 0;
	drv_dummy.pread = 0;
	drv_dummy.ioctl = 0;
	*/
}

void dump_drivers( void ){
	unsigned int i;
	for ( i = 0; i < driver_p; i++ ){
		printf( "driver %d: id=0x%x, type=", i, driver_list[i].id );
		if ( driver_list[i].type & USER_IN )  printf("Input (user), ");
		if ( driver_list[i].type & USER_OUT ) printf("Output (user), ");
		if ( driver_list[i].type & DISK )     printf("Disk, ");
		if ( driver_list[i].type & FILE_S )   printf("File system, ");
		printf( "\n" );
	}
}
	
#endif
