#ifndef _kernel_stdio_c
#define _kernel_stdio_c
#include <stdio.h>

int printf( char *format, ... ){
	int slen = strlen( format ), i = 0, signed_int;
	int fd;
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

int kfprintf( int fd, char *format, ... ){
	va_list args;
	va_start( args, format );

	kvfprintf( fd, format, args );

	return 0;
}

int kprintfd( char *format, ... ){
	int debug = open( "/dev/ser0", O_APPEND );
	va_list args;
	va_start( args, format );

	kvfprintf( debug, format, args );
	close( debug );

	return 0;
}

int kvfprintf( int fd, char *format, va_list args ){
	int slen = strlen( format ), i = 0, signed_int;
	unsigned int unsigned_int;
	char buf, *str;

	for ( i = 0; i < slen; i++ ){
		if ( format[i] == '%' ){
			switch( format[++i] ){
				case '%':
					buf = '%';
					write( fd, &buf, 1 );
					break;
				case 'c':
					buf = va_arg( args, int );
					write( fd, &buf, 1 );
					break;
				case 's':
					str = va_arg( args, char * );
					write( fd, str, strlen( str ));
					break;
				case 'd':
					signed_int = va_arg( args, int );
					fprint_num( fd, signed_int );
					break;
				case 'u':
					unsigned_int = va_arg( args, unsigned int );
					fprint_num( fd, unsigned_int );
					break;
				case 'x':
					unsigned_int = va_arg( args, unsigned int );
					fprint_hex( fd, unsigned_int );
					break;
			}
		} else {
			//kputchar( format[i] );
			write( fd, &format[i], 1 );
		}
	}
	va_end( args );
	return 0;
}

void fprint_num( int fd, unsigned long input ){
	unsigned int a = input, r = a, d = 0, i = 0;
	char 	buf[16],
		temp = '0';

	if ( !input ) 
		write( fd, &temp, 1 );

	for ( i = 0 ; a; r = a = d ){
		d = r / 10;
		buf[i++] = (r - d * 10) + '0';
	}

	while ( i-- ) 
		write( fd, &buf[i], 1 );
}

void fprint_hex( int fd, unsigned long input ){
	unsigned long a = input, r = a, d = 0, i = 0;
	char 	letters[] = "0123456789abcdef",
		buf[16],
		temp = '0';

	if ( !input ) 
		write( fd, &temp, 1 );
	
	for ( i = 0; a; r = a = d ){
		d = r/16;
		buf[i++] = letters[ r - d * 16 ];
	};

	while ( i-- ) 
		write( fd, &buf[i], 1 );
}

void print_num( unsigned long input ){
	unsigned int a = input, r = a, d = 0, i = 0;
	char 	buf[16];

	if ( !input ) 
		kputchar( '0' );

	for ( i = 0 ; a; r = a = d ){
		d = r / 10;
		buf[i++] = (r - d * 10) + '0';
	}

	while ( i-- ) 
		kputchar( buf[i] );
}

void print_hex( unsigned long input ){
	unsigned long a = input, r = a, d = 0, i = 0;
	char 	letters[] = "0123456789abcdef",
		buf[16];

	if ( !input ) 
		kputchar( '0' );
	
	for ( i = 0; a; r = a = d ){
		d = r/16;
		buf[i++] = letters[ r - d * 16 ];
	};

	while ( i-- ) 
		 kputchar( buf[i] );
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
