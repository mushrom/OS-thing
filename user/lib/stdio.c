#ifndef _user_stdio_c
#define _user_stdio_c
#include <stdio.h>
#define stdin  0
#define stdout 1
#define stderr 2

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
					buf = '%';
					syscall_write( stdout, &buf, 1 );
					break;
				case 'c':
					buf = va_arg( args, int );
					syscall_write( stdout, &buf, 1 );
					break;
				case 's':
					str = va_arg( args, char * );
					syscall_write( stdout, str, strlen( str ));
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
			syscall_write( stdout, &format[i], 1 );
		}
	}
	va_end( args );
	return 0;
}

void print_num( unsigned long input ){
	unsigned int a = input, r = a, d = 0, i = 0;
	char buf[16], temp;

	if ( !input ){
		temp = '0';
		syscall_write( stdout, '0', 1 );
	}

	for ( i = 0 ; a; r = a = d ){
		d = r / 10;
		buf[i++] = (r - d * 10) + '0';
	}
	while ( i-- ) syscall_write( stdout, &buf[i], 1 );
}

void print_hex( unsigned long input ){
	unsigned long a = input, r = a, d = 0, i = 0;
	char 	letters[] = "0123456789abcdef",
		temp, 
		buf[16];

	if ( !input ){
		temp = '0';
		syscall_write( stdout, '0', 1 );
	}
	
	for ( i = 0; a; r = a = d ){
		d = r/16;
		buf[i++] = letters[ r - d * 16 ];
	};
	while ( i-- ) syscall_write( stdout, &buf[i], 1 );
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
