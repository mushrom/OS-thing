#ifndef _kernel_string_h
#define _kernel_string_h
#include <alloc.h>

unsigned int strlen( char * );
int strcmp( char *, char * );
void *memset( void *, unsigned char, unsigned int );
void *memsetw( void *, unsigned char, unsigned int );
void *memcpy( void *, void *, unsigned int );
void *memmove( void *, void *, unsigned int );

#endif
