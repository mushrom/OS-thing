#ifndef _kernel_skio_h
#define _kernel_skio_h

#define IN_BUF_SIZE 32
#define insl( port, buffer, count )\
	asm volatile ( "cld; rep; insl" :: "D"(buffer), "d"(port), "c"(count))

void pkputs( char *, unsigned char );
void pkputc ( char, unsigned char );

unsigned char inb( unsigned short );
void outb( unsigned short, unsigned char );
void outl( unsigned short, unsigned long );

unsigned char get_in_char( void );
void put_in_char( unsigned char );
void pause( void );

#endif
