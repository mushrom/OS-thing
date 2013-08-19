#ifndef _kernel_serial_h
#define _kernel_serial_h
#include <isr.h>
#include <skio.h>
#include <fs.h>
#include <devfs.h>

file_system_t *serial_create( );

#endif
