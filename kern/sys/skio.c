#ifndef _kernel_skio_c
#define _kernel_skio_c

#include <sys/skio.h>

/* primitive screen output functions 
void pkputs( char *input, unsigned char color ){
        int i, j;
        for ( i = 0, j = 0; input[j] != '\0'; i+=2, j++ ){
                videoram[i] = input[j];
                videoram[i+1] = color;
        }
        return;
}

void pkputc ( char input, unsigned char color ){
        videoram[0] = input;
        videoram[1] = color;
        return;
}*/

unsigned int strlen( char *input ){
	int i;
	for ( i = 0; input[i] != '\0'; i++ );
	return i;
}

/* inports/outports */
unsigned char inportb( unsigned short port ){
        unsigned char ret;
        asm volatile( "inb %1, %0"
                : "=a"(ret) : "Nd"(port) );
        return ret;
}

void outportb( unsigned short _port, unsigned char _data ){
	asm volatile( "outb %1, %0" : : "dN" (_port), "a" (_data));
}	

/* basic memory stuffs */
void *memset( void *dest, unsigned char value, unsigned int count ){
	char *ret_dest = dest;
	while ( count-- ){
		*(ret_dest++) = value;
	}
	return dest;
}

void *memsetw( void *dest, unsigned char value, unsigned int count ){
	char *ret_dest = dest;
	while ( count-- ){
		*(ret_dest++) = value;
	}
	return dest;
}

void *memcpy( void *dest, void *src, unsigned int count ){
	char *ret_dest = dest;
	char *src_dest = src;
	while ( count-- ){
		*(ret_dest++) = *(src_dest++);
	}
	return dest;
}

void *memmove( void *dest, void *src, unsigned int count ){
	char *ret_dest = dest;
	char *src_dest = src;
	while ( count-- ){
		*(ret_dest++) = *(src_dest++);
		*(src_dest) = 0;
	}
	return dest;
}

#endif
