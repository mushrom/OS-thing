#ifndef _user_string_h
#define _user_string_h

unsigned int strlen( char * );
int strcmp( char *, char * );
void *memset( void *, unsigned char, unsigned int );
void *memsetw( void *, unsigned char, unsigned int );
void *memcpy( void *, void *, unsigned int );
void *memmove( void *, void *, unsigned int );
char *strcpy( char *, char * );

#endif
