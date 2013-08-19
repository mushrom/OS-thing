#ifndef _kernel_keyboard_c
#define _kernel_keyboard_c
#include "kb.h"

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

static void keyboard_handler( registers_t *regs ){
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

static int open_kb( file_node_t *node, char *path, int flags ){
	printf( "[kb] got here\n" );
	return 1;
}

static int close_kb( file_node_t *node ){
	return 0;
}

static int read_kb( file_node_t *node, void *buf, size_t size ){
	unsigned int i;
	char *out_buf = buf;
	read_buf = 0;
	for ( i = 0; i < size; i++ ){
		while ( !read_buf ){ sleep_thread(3);}
		out_buf[i] = read_buf;
		read_buf = 0;
	}
	read_buf = 0;
	return i;
}

static int pread_kb( file_node_t *node, void *buf, size_t size, unsigned long offset ){
	return read_kb( node, buf, size );
}

void unload_keyboard( void ){
	unregister_interrupt_handler( IRQ1 );
}

static int kb_unsupported( file_node_t *node ){
	return -ENOTSUP;
}

static int kb_get_info( file_node_t *node, file_info_t *buf ){
	buf->type 	= FS_CHAR_D;
	buf->name	= "kb";
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

file_funcs_t kb_funcs = { 
	.read	= read_kb,
	.pread	= pread_kb,

	.write	= kb_unsupported,
	.pwrite	= kb_unsupported,
	.ioctl	= kb_unsupported,
	.mkdir	= kb_unsupported,
	.mknod	= kb_unsupported,
	.link	= kb_unsupported,
	.unlink	= kb_unsupported,

	.open	= open_kb,
	.close	= close_kb,

	.get_info = kb_get_info,
};

file_system_t *keyboard_create( ){
	file_system_t *kb_driver = knew( file_system_t );

	kb_driver->name = "ps2keyboard";
	kb_driver->id = 321;
	kb_driver->ops = &kb_funcs;
	kb_driver->i_root = 0;
	kb_driver->fs_data = 0;

	register_interrupt_handler( IRQ1, &keyboard_handler );

	/** Kludge alert; for some reason the keyboard will occassionally
 	 * (read: frequently) not interrupt if the handler isn't called immediately.
 	 * I have no idea why, but if it works... */
	keyboard_handler( 0 );

	return kb_driver;
}

#endif
