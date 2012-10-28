#ifndef _kernel_skio_h
#define _kernel_skio_h

#define IN_BUF_SIZE 64

void pkputs( char *, unsigned char );
void pkputc ( char, unsigned char );

unsigned char get_in_char( void );
void put_in_char( unsigned char );
void pause( void );
void reboot( void );

#define insl( port, buffer, count )\
	asm volatile ( "cld; rep; insl" :: "D"(buffer), "d"(port), "c"(count))

unsigned char inb( unsigned short port );
void outb( unsigned short _port, unsigned char _data );
void outl( unsigned short _port, unsigned long _data );

#endif
