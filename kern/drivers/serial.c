#ifndef _kernel_serial_c
#define _kernel_serial_c
#include "serial.h"
/* based on source from wiki osdev.org */

#define PORT 0x3f8

int serial_received( void ){
	return inb( PORT + 5 ) & 1;
}

int transmit_empty( void ){
	return inb( PORT + 5 ) & 0x20;
}

static int read_serial( file_node_t *node, void *buf, size_t size ){
	char *out_buf = buf;
	unsigned int i;

	for ( i = 0; i < size; i++ ){
		while ( serial_received( ) == 0 )
			sleep_thread( 3 );

		out_buf[i] = inb( PORT );
	}

	return i;
}

static int write_serial( file_node_t *node, void *buf, unsigned long size ){
	char *in_buf = buf;
	unsigned int i;

	for ( i = 0; i < size; i++ ){
		while ( transmit_empty( ) == 0 )
			sleep_thread( 3 );

		outb( PORT, in_buf[i] );
	}

	return i;
}

static int pread_serial( file_node_t *node, void *buf, size_t size, unsigned long offset ){
	return read_serial( node, buf, size );
}

static int pwrite_serial( file_node_t *node, void *buf, size_t size, unsigned long offset ){
	return write_serial( node, buf, size );
}

void unload_serial( void ){
}

void init_serial( void ){
	file_node_t serial_driver;

	outb( PORT + 1, 0x00 ); // disable interrupts
	outb( PORT + 3, 0x80 ); // enable dlab (set baud rate divisor)
	outb( PORT + 0, 0x03 ); // set divisor to 3 	(low) 38400 baud
	outb( PORT + 1, 0x00 ); //			(high)
	outb( PORT + 3, 0x03 ); // 8 bits, no parity, one stop bit
	outb( PORT + 2, 0xc7 ); // enable fifo, clear them, with 14-byte threshold
	outb( PORT + 4, 0x0b ); // irqs enabled, rts/dsr set

	memset( &serial_driver, 0, sizeof( file_node_t ));
	memcpy( serial_driver.name, "ser0", 5 );

	serial_driver.type	= FS_CHAR_D;
	serial_driver.read	= read_serial;
	serial_driver.pread	= pread_serial;
	serial_driver.write	= write_serial;
	serial_driver.pwrite	= pwrite_serial;

	devfs_register_device( serial_driver );

}

#endif
