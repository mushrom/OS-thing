#ifndef _kernel_stdio_h
#define _kernel_stdio_h
#include <sys/skio.h>
#include <lib/stdint.h>
#include <lib/stdarg.h>
#include <lib/string.h>
#include <drivers/console.h>

typedef unsigned long size_t;

int printf( char *, ... );
void print_num( unsigned long input );
void print_hex( unsigned long input );
int atoi( char * );

#endif
