#ifndef _user_stdio_h
#define _user_stdio_h
#include <stdarg.h>

#define stdin 0
#define stdout 1
#define stderr 2

typedef unsigned long size_t;

int printf( char *, ... );
void print_num( unsigned long input );
void print_hex( unsigned long input );
int atoi( char * );

#endif
