#ifndef _user_stdio_h
#define _user_stdio_h
#include <stdarg.h>

#define stdin 0
#define stdout 1
#define stderr 2

typedef unsigned long size_t;

int printf( char *, ... );
void putc( int c, int fd );
void putchar( int c );
int getc( int fd );
int getchar( );
void print_num( unsigned long input );
void print_hex( unsigned long input );
int atoi( char * );

#endif
