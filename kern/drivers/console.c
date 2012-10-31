#ifndef _kernel_console_c
#define _kernel_console_c

#include "console.h"
//static int console_write( int screen, void *buf, uint32_t size );

char *videoram = (char *)VIDEORAM;
unsigned short int cur_x_pos = 0,
		   cur_y_pos = 0,
		   color = 0x0f,
		   con_scroll_offset = 0;
//console_driver.write = console_write;

void cls( void ){
	memset( videoram, 0, 4000 );
	cur_x_pos = cur_y_pos = 0;
}

void move_cursor( ){
	unsigned temp;
        temp = (cur_y_pos * 80) + cur_x_pos;  //+ (cur_x_pos / 2);
	//temp = 10;

	outb( 0x3d4, 14 );
	outb( 0x3d5, temp >> 8 );
	outb( 0x3d4, 15 );
	outb( 0x3d5, temp );

}

void kcheck_scroll( void ){
	unsigned short int i;
	if ( cur_y_pos > 24 ){
		for ( i = con_scroll_offset; i < 24; i++ ){
			memcpy( videoram+(i*XSIZE*2), videoram+((i+1)*XSIZE*2), XSIZE*2 );
		}
		for ( ; cur_y_pos > 24; cur_y_pos-- );
		memset( videoram+(i*XSIZE*2), 0, XSIZE*2 );
	}
}

void kputchar( unsigned char input ){
	unsigned int pos = (( cur_y_pos * XSIZE ) + cur_x_pos );

	if ( input >= 0x20 && input < 0x7f ){
		videoram[pos*2] = input;
		videoram[pos*2+1] = color;
		videoram[pos*2+3] = color;
		cur_x_pos++;

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
		cur_x_pos += 8 - ( cur_x_pos % 8 );
	} else if ( input == '\b' ) {
		cur_x_pos --;
		videoram[pos*2-2] = ' ';
	} else if ( input >= 0x10 && input <= 0x1f ){
		set_color( input - 0x10 );
	}
	move_cursor();
	kcheck_scroll();
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
static int console_write( file_node_t *node, void *buf, unsigned long size ){
	char *in_buf = buf;
	int i = 0;
	for ( i = 0; i < size; i++ ){
		kputchar( in_buf[i] );
	}
	return i;
}

static int console_pwrite( file_node_t *node, void *buf, unsigned long size, unsigned long offset ){
	return console_write( node, buf, size );
}

void init_console(){
	file_node_t console_driver;

	memset( &console_driver, 0, sizeof( file_node_t ));
	memcpy( console_driver.name, "tty", 4 );
	console_driver.type	= FS_CHAR_D;
	console_driver.write	= console_write;
	console_driver.pwrite	= console_pwrite;

	devfs_register_device( console_driver );
}

#endif
