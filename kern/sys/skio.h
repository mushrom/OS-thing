#ifndef _kernel_skio_h
#define _kernel_skio_h

#define IN_BUF_SIZE 64

void pkputs( char *, unsigned char );
void pkputc ( char, unsigned char );

unsigned char get_in_char( void );
void put_in_char( unsigned char );
void pause( void );

#endif
