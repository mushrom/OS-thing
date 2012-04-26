#ifndef _kernel_skio_h
#define _kernel_skio_h
unsigned char *videoram = (unsigned char *) 0xb8000;

//primitive screen output functions
void pkputs( char *input, unsigned short int color ){
        int i, j;
        for ( i = 0, j = 0; input[j] != '\0'; i+=2, j++ ){
                videoram[i] = input[j];
                videoram[i+1] = color;
        }
        return;
}

void pkputc ( char input, unsigned short color ){
        videoram[0] = input;
        videoram[1] = color;
        return;
}

unsigned int strlen( char *input ){
	int i;
	for ( i = 0; input[i] != '\0'; i++ );
	return i;
}

//inports/outports
unsigned char inportb( unsigned short port ){
        unsigned char ret;
        asm volatile( "inb %1, %0"
                : "=a"(ret) : "Nd"(port) );
        return ret;
}

void outportb( unsigned short _port, unsigned char _data ){
	asm volatile( "outb %1, %0" : : "dN" (_port), "a" (_data));
}	

//basic memory stuffs
unsigned char memset( unsigned char *dest, unsigned char value, unsigned int count ){
	unsigned char *ret_dest = dest;
	while ( count-- ){
		*(dest++) = value;
	}
	return *ret_dest;
}

unsigned char memsetw( unsigned short *dest, unsigned short value, unsigned int count ){
	unsigned short *ret_dest = dest;
	while ( count-- ){
		*(dest++) = value;
	}
	return *ret_dest;
}

unsigned char memcpy( unsigned char *dest, unsigned char *src, unsigned int count ){
	unsigned char *ret_dest = dest;
	while ( count-- ){
		*(dest++) = *(src++);
	}
	return *ret_dest;
}

unsigned char memmove( unsigned char *dest, unsigned char *src, unsigned int count ){
	unsigned char *ret_dest = dest;
	while ( count-- ){
		*(dest++) = *(src++);
		*(src) = 0;
	}
	return *ret_dest;
}
#endif
