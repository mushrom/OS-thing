#ifndef _kernel_keyboard_c
#define _kernel_keyboard_c
#include <drivers/kb.h>

#define KB_SH 1 /* S = shift */

unsigned int shift_on = 0;
unsigned char read_buf = 0;
unsigned char map_set  = 0;

unsigned char us_kbd[128] = {
	 0,   27, 
	'1', '2', '3',  '4', '5', '6', '7', '8', '9', '0',
	'-', '=','\b', '\t',
	'q', 'w', 'e',  'r', 't',  'y', 'u', 'i',  'o', 'p',  '[',  ']', '\n',    0,
	'a', 's', 'd',  'f', 'g',  'h', 'j', 'k',  'l', ';', '\'',  '`',KB_SH, '\\', 
	'z', 'x', 'c',  'v', 'b',  'n', 'm', ',',  '.', '/',KB_SH,  '*',    0,  ' ',
	  0,   0,   0,    0,   0,    0,   0,   0,    0,   0,    0,    0,    0,    0,
      KB_UA,   0, '-',KB_LA,   0,KB_RA, '+',   0,KB_DA,   0,    0,    0,    0,    0,
	  0,   0,   0,    0,   0
};

unsigned char us_kbd_shift[128] = {
	 0,   27, 
	'!', '@', '#',  '$', '%', '^', '&', '*', '(', ')',
	'_', '+','\b', '\t',
	'Q', 'W', 'E',  'R', 'T',  'Y', 'U', 'I',  'O', 'P',  '{',  '}', '\n',    0,
	'A', 'S', 'D',  'F', 'G',  'H', 'J', 'K',  'L', ':', '\"',  '~',KB_SH,  '|', 
	'Z', 'X', 'C',  'V', 'B',  'N', 'M', '<',  '>', '?',KB_SH,  '*',    0,  ' ',
	  0,   0,   0,    0,   0,    0,   0,   0,    0,   0,    0,    0,    0,    0,
      KB_UA,   0, '-',KB_LA,   0,KB_RA, '+',   0,KB_DA,   0,    0,    0,    0,    0,
	  0,   0,   0,    0,   0
};

unsigned char 	*lower_map = us_kbd, 
		*upper_map = us_kbd_shift;
unsigned char 	*map;

static void keyboard_handler( registers_t regs ){
	unsigned char scancode;
	unsigned char buf;
	if ( !map_set ){
		map = lower_map;
		map_set = 1;
	}
	scancode = inb( 0x60 );
	if ( scancode & 0x80 ){
		buf = map[ scancode - 0x80 ];
		if ( buf == KB_SH ){
			map = lower_map;
		} 
	} else {
		buf = map[ scancode ];
		if ( buf == KB_SH ){
			map = upper_map;
		} else {
			/* This doesn't draw it on screen, it pushes it into an 
			 * input buffer defined in sys/skio.h */
			put_in_char( buf );
			read_buf = buf;
		}
	}
}

/* Just a note, I wouldn't recommend directly reading the keyboard for general purpose reading. 
 * It's a waste of resources to constantly poll, especially when it's already pushing
 * to a buffer on interrupt. This is here just to fit with the driver model. */

static int read_kb( file_node_t *node, void *buf, size_t size ){
	unsigned int i;
	char *out_buf = buf;
	read_buf = 0;
	for ( i = 0; i < size; i++ ){
		while ( !read_buf );
		out_buf[i] = read_buf;
		read_buf = 0;
	}
	return i;
}

void unload_keyboard( void ){
	unregister_interrupt_handler( IRQ1 );
}

void init_keyboard( void ){

	file_node_t kb_driver;
	memset( &kb_driver, 0, sizeof( file_node_t ));
	memcpy( kb_driver.name, "kb0", 4 );
	kb_driver.type	= FS_CHAR_D;
	kb_driver.read	= read_kb;

	devfs_register_device( kb_driver );
	register_interrupt_handler( IRQ1, &keyboard_handler );
}

#endif
