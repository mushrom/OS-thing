#ifndef _kernel_skio_c
#define _kernel_skio_c
#include <sys/skio.h>

unsigned char input_buf[ IN_BUF_SIZE ];
unsigned int  input_buf_p = 0, input_buf_i = 0;
unsigned int  got_input = 0, i;

/* inports/outports */
/*
unsigned char inb( unsigned short port ){
        unsigned char ret;
        asm volatile( "inb %1, %0" : "=a"(ret) : "Nd"(port) );
        return ret;
}

void outb( unsigned short _port, unsigned char _data ){
	asm volatile( "outb %1, %0" : : "dN" (_port), "a" (_data));
}	

void outl( unsigned short _port, unsigned long _data ){
	asm volatile( "outl %1, %0" : : "dN" (_port), "a" (_data));
}	
*/

/* manipulate in/out buffers */
unsigned char get_in_char( void ){
	if ( got_input ){
		input_buf_i++;
		input_buf_i %= IN_BUF_SIZE;
		got_input--;
		return input_buf[ input_buf_i ];
	} else {
		return 0;
	}
}

void put_in_char( unsigned char buf ){
	input_buf_p++;
	got_input++;
	input_buf_p %= IN_BUF_SIZE;

	input_buf[ input_buf_p ] = buf;
}

void pause( void ){
	while ( !got_input );
}


#endif
