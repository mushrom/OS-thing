#ifndef _kernel_itoa_c
#define _kernel_itoa_c
#include <itoa.h>
#include <sys/console.h>

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
#endif
