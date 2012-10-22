#ifndef _kernel_serial_console_h
#define _kernel_serial_console_h

#define S_BASE 0x16000000
#define SF_REG 0x18
#define SBUF_FULL ( 1 << 5 )

void kputchar( char c );
void kputs( char *str );

#endif
