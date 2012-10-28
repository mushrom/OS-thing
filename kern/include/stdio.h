#ifndef _kernel_stdio_h
#define _kernel_stdio_h
#include <skio.h>
#include <stdint.h>
#include <stdarg.h>
#include <string.h>
#include <console.h>

typedef unsigned long size_t;

int printf( char *, ... );
void print_num( unsigned long input );
void print_hex( unsigned long input );
int atoi( char * );

#endif
