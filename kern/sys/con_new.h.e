#include "skio.h"
#ifndef _kernel_console_h
#define _kernel_console_h
#define XSIZE 80 /*Actual terminal size is 80x25, this is 160 because the videoram needs 16 bit input*/
#define YSIZE 25

struct _kcon_char {
	unsigned char value;
	unsigned char color;
};

unsigned short int cur_x_pos = 0,
		   cur_y_pos = 0,
		   color = 0x0f;
struct _kcon_char *videoram = ( struct _kcon_char *)0xb8000;
void cls( void ){
	memset( ( char *)videoram, 0, 2000 );
	cur_x_pos = cur_y_pos = 0;
}

void _kcheck_scroll( void ){
	unsigned short int i, j;
	if ( cur_y_pos > 24 ){
		for ( i = 0; i < 24; i++ ){
			for ( j = 0; j < XSIZE; j++ )
				videoram[i*j] = videoram[(i+1)*j];
		}
		for ( ; cur_y_pos > 24; cur_y_pos-- );
		for ( j = 0; j < XSIZE; j++ ){
			videoram[i * XSIZE].value = 0;
			videoram[i * XSIZE].color = 0;
		}
	}
}

void kputchar( unsigned char input ){
	unsigned int pos;
	if ( input >= 0x20 && input < 0x7f ){
		pos = (( cur_y_pos * XSIZE ) + cur_x_pos );
		
		videoram[pos].value = input;
		videoram[pos].color = color;
		cur_x_pos++;

		if ( cur_x_pos > XSIZE ){
			cur_y_pos++;
			cur_x_pos = 0;
		}
	} else if ( input == 0x0a ) {
		cur_y_pos++;
		cur_x_pos = 0;
	} else if ( input == 0x0d ) {
		cur_x_pos = 0;
	}
	_kcheck_scroll();
}

void kputs( char *input ){
	unsigned short int i;
	for ( i = 0; i < strlen(input); i++ ){
		kputchar( input[i] );
	}
}
#endif
