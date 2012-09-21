#ifndef _kernel_keyboard_c
#define _kernel_keyboard_c
#include <sys/kb.h>

#define S 1 /* S = shift */

unsigned int shift_on = 0;

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

	scancode = inb( 0x60 );
	if ( scancode & 0x80 ){
		buf = us_kbd[ scancode - 0x80 ];
		if ( buf == S ){
			shift_on = 0;
		} 
	} else {
		buf = us_kbd[ scancode ];
		if ( buf == S ){
			shift_on = 1;
		} else {
			if ( shift_on ){
				buf -= 'a'-'A';
			}
			/* This doesn't draw it on screen, it pushes it into an 
			 * input buffer defined in sys/skio.h */
			put_in_char( buf );
		}
	}
}


void init_keyboard( void ){
	register_interrupt_handler( IRQ1, &keyboard_handler );
}

#endif
