#ifndef _kernel_skio_h
#define _kernel_skio_h

void pkputs( char *, unsigned char );
void pkputc ( char, unsigned char );
unsigned int strlen( char * );
unsigned char inportb( unsigned short );
void outportb( unsigned short, unsigned char );
void *memset( void *, unsigned char, unsigned int );
void *memsetw( void *, unsigned char, unsigned int );
void *memcpy( void *, void *, unsigned int );
void *memmove( void *, void *, unsigned int );

#endif
