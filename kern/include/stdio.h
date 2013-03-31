#ifndef _kernel_stdio_h
#define _kernel_stdio_h
#include <skio.h>
#include <stdint.h>
#include <stdarg.h>
#include <string.h>
#include <console.h>

typedef unsigned long size_t;

extern int debug_file;
extern int stdout_file;

//int keprintf( char *, ... );
int printf( char *, ... );
int kfprintf( int fd, char *, ... );
int kvfprintf( int fd, char *, va_list );
void print_num( unsigned long input );
void print_hex( unsigned long input );
void fprint_num( int fd, unsigned long input );
void fprint_hex( int fd, unsigned long input );
int atoi( char * );

#endif
