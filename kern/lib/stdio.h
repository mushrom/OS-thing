#ifndef _kernel_stdio_h
#define _kernel_stdio_h
#include <drivers/console.h>
#include <drivers/driver.h>
#include <sys/skio.h>
#include <lib/stdint.h>
#include <lib/stdarg.h>
#include <lib/string.h>

int printf( char *, ... );
void putchar( char buf );
void puts( char *str );
void print_num( unsigned long input );
void print_hex( unsigned long input );

#endif
