#ifndef _kernel_stdio_c
#define _kernel_stdio_c
#include <lib/stdio.h>

int printf( char *format, ... ){
	int slen = strlen( format ), i = 0, signed_int;
	unsigned int unsigned_int;
	char buf, *str;
	va_list args;
	va_start( args, format );

	for ( i = 0; i < slen; i++ ){
		if ( format[i] == '%' ){
			switch( format[++i] ){
				case '%':
					kputchar( '%' );
					break;
				case 'c':
					buf = va_arg( args, int );
					kputchar( buf );
					break;
				case 's':
					str = va_arg( args, char * );
					kputs( str );
					break;
				case 'd':
					signed_int = va_arg( args, int );
					print_num( signed_int );
					break;
				case 'u':
					unsigned_int = va_arg( args, unsigned int );
					print_num( unsigned_int );
					break;
				case 'x':
					unsigned_int = va_arg( args, unsigned int );
					print_hex( unsigned_int );
					break;
			}
		} else {
			kputchar( format[i] );
		}
	}
	va_end( args );
	return 0;
}

void print_num( unsigned long input ){
	unsigned int a = input, r = a, d = 0, i = 0;
	char buf[16];

	if ( !input ) kputchar('0');

	for ( i = 0 ; a; r = a = d ){
		d = r / 10;
		buf[i++] = (r - d * 10) + '0';
	}
	while ( i-- ) kputchar( buf[i] );
}

void print_hex( unsigned long input ){
	unsigned long a = input, r = a, d = 0, i = 0;
	char 	letters[] = "0123456789abcdef",
		buf[16];

	if ( !input ) kputchar('0');
	
	for ( i = 0; a; r = a = d ){
		d = r/16;
		buf[i++] = letters[ r - d * 16 ];
	};
	while ( i-- ) kputchar( buf[i] );
}

int atoi( char *string ){
	unsigned int 	output = 0, 
			i = strlen( string ) - 1, 
			p = 1;

	for ( ; i + 1; i-- ){
		output += ( string[i] - '0' ) * p;
		p *= 10;
	}
	return output;
}
#endif
