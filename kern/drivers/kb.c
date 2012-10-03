#ifndef _kernel_keyboard_c
#define _kernel_keyboard_c
#include <drivers/kb.h>

#define S 1 /* S = shift */

unsigned int shift_on = 0;
unsigned char read_buf = 0;

unsigned char us_kbd[128] = {
	 0,   27, 
	'1', '2', '3', '4', '5', '6', '7', '8', '9', '0',
	'-', '=','\b','\t',
	'q', 'w', 'e', 'r', 't', 'y', 'u', 'i', 'o', 'p', '[', ']', '\n', 0,
	'a', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l', ';','\'', '`',  S, '\\', 
	'z', 'x', 'c', 'v', 'b', 'n', 'm', ',', '.', '/',  S,  '*',  0,  ' ',
	  0,   0,   0,   0,   0,   0,   0,   0,   0,   0,  0,    0,  0,    0,
	  0,   0,   0, '-',   0,   0,   0, '+',   0,   0,  0,    0,  0,    0,
	  0,   0,   0,   0,   0
};

static void keyboard_handler( registers_t regs ){
	unsigned char scancode;
	unsigned char buf;
	unsigned char *map = us_kbd;

	scancode = inb( 0x60 );
	if ( scancode & 0x80 ){
		buf = map[ scancode - 0x80 ];
		if ( buf == S ){
			shift_on = 0;
		} 
	} else {
		buf = map[ scancode ];
		if ( buf == S ){
			shift_on = 1;
		} else {
			if ( shift_on ){
				buf -= 'a'-'A';
			}
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
static void read_kb( int d, void *buf, size_t size ){
	unsigned int i;
	char *out_buf = buf;
	for ( i = 0; i < size; i++ ){
		while ( !read_buf );
		out_buf[i] = read_buf;
		read_buf = 0;
	}
}

void unload_keyboard( void ){
	unregister_interrupt_handler( IRQ1 );
}

void init_keyboard( void ){
	register_interrupt_handler( IRQ1, &keyboard_handler );
/*
	kernel_driver_t kb_driver;

	memcpy( kb_driver.name, "keyboard", 9 );
	kb_driver.id		= 0xfee1dead;
	kb_driver.type 		= USER_IN;
	kb_driver.init		= 0;
	kb_driver.write		= 0;
	kb_driver.read		= (read_func)read_kb;
	kb_driver.pwrite	= 0;
	kb_driver.pread		= 0;
	kb_driver.ioctl		= 0;
	kb_driver.unload	= (unload_func)unload_keyboard;
	//kb_driver.unload	= 0;

	register_driver( kb_driver );
*/
}

#endif
