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

static int console_open( file_node_t *node, void *buf, unsigned long size ){
	return 1;
}

static int console_close( file_node_t *node ){
	return 0;
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
	//printf( "[pwrite 0x%x\n", buf );
	return console_write( node, buf, size );
}

static int console_unsupported( file_node_t *node ){
	return -ENOTSUP;
}

static int console_get_info( file_node_t *node, file_info_t *buf ){
	buf->type 	= FS_CHAR_D;
	buf->name	= "tty";
	buf->mask	= 0777;
	buf->uid	= 0;
	buf->gid	= 0;
	buf->time	= 0;
	buf->inode	= 0;
	buf->size	= 0;
	buf->links	= 1;
	buf->flags	= 1;
	buf->fs		= node->fs;
	buf->mount_id	= 0;

	return 0;
}

file_funcs_t console_funcs = {
	.read 	= console_unsupported,
	.pread 	= console_unsupported,
	.write	= console_write,
	.pwrite	= console_pwrite,
	.ioctl	= console_unsupported,

	.mkdir	= console_unsupported,
	.mknod	= console_unsupported,
	.link	= console_unsupported,
	.unlink = console_unsupported,

	.open	= console_open,
	.close	= console_close,

	.get_info = console_get_info,
};

file_system_t *console_create( ){
	file_system_t *console_driver = knew( file_system_t );

	console_driver->name	= "textconsole";
	console_driver->id	= 123;
	console_driver->ops	= &console_funcs;
	console_driver->i_root	= 0;
	console_driver->fs_data	= 0;

	return console_driver;
}

#endif
