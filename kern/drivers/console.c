#ifndef _kernel_console_c
#define _kernel_console_c

#include <sys/console.h>
//static int console_write( int screen, void *buf, uint32_t size );

char *videoram = (char *)VIDEORAM;
unsigned short int cur_x_pos = 0,
		   cur_y_pos = 0,
		   color = 0x0f;
//console_driver.write = console_write;

void cls( void ){
	memset( videoram, 0, 4000 );
	cur_x_pos = cur_y_pos = 0;
}


void _kcheck_scroll( void ){
	unsigned short int i;
	if ( cur_y_pos > 24 ){
		for ( i = 0; i < 24; i++ ){
			memcpy( videoram+(i*XSIZE), videoram+((i+1)*XSIZE), XSIZE );
		}
		for ( ; cur_y_pos > 24; cur_y_pos-- );
		memset( videoram+(i*XSIZE), 0, XSIZE );
	}
}

void kputchar( unsigned char input ){
	unsigned int pos = (( cur_y_pos * XSIZE ) + cur_x_pos );

	if ( input >= 0x20 && input < 0x7f ){
		videoram[pos] = input;
		videoram[pos+1] = color;
		cur_x_pos+=2;

		if ( cur_x_pos > XSIZE ){
			cur_y_pos++;
			cur_x_pos = 0;
		}
	} else if ( input == '\n' ) {
		cur_y_pos++;
		cur_x_pos = 0;
	} else if ( input == '\r' ) {
		cur_x_pos = 0;
	} else if ( input == '\t' ) {
		cur_x_pos += 16 - ( cur_x_pos % 16 );
	} else if ( input == '\b' ) {
		cur_x_pos -= 2;
		videoram[pos-2] = 0;
	} else if ( input >= 0x10 && input <= 0x1f ){
		set_color( input - 0x10 );
	}
	_kcheck_scroll();
}

void kputs( char *input ){
	unsigned short int i;
	for ( i = 0; i < strlen(input); i++ ){
		kputchar( input[i] );
	}
}

void set_color( unsigned char new_color ){
	color = new_color;
}

/* Driver interface */
/* Commented out until devfs is implemented
static int console_write( int screen, void *buf, uint32_t size ){
	char *in_buf = buf;
	int i = 0;
	for ( i = 0; i < size; i++ ){
		kputchar( in_buf[i] );
	}
	return i;
}
*/

void init_console(){
/*
	kernel_driver_t console_driver;

	memcpy( console_driver.name, "console", MAX_NAME );
	console_driver.id     = 0x5eeca7;
	console_driver.type   = USER_OUT;
	console_driver.init   = 0;
	console_driver.write  = (write_func)console_write;
	console_driver.read   = 0;
	console_driver.pwrite = 0;
	console_driver.pread  = 0;
	console_driver.ioctl  = 0;
	console_driver.unload = 0;

	register_driver( console_driver );
*/
}

#endif
