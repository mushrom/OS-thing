#ifndef _kernel_serial_console_c
#define _kernel_serial_console_c

#define S_BASE 0x16000000
#define SF_REG 0x18
#define SBUF_FULL ( 1 << 5 )

void kputchar( char c ){
	while (*(unsigned long *)( S_BASE + SF_REG + SBUF_FULL ));
	*(unsigned long*)S_BASE = c;
}

void kputs( char *str ){
	while ( *str ) kputchar( *str++ );
}

#endif
