#ifndef _kernel_console_c
#define _kernel_console_c

#include <sys/console.h>

char *videoram = (char *)VIDEORAM;
unsigned short int cur_x_pos = 0,
		   cur_y_pos = 0,
		   color = 0x0f;

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
	unsigned int pos;
	if ( input >= 0x20 && input < 0x7f ){
		pos = (( cur_y_pos * XSIZE ) + cur_x_pos );
		
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
#endif
