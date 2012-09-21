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

#endif
