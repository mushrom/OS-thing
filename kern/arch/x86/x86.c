#ifndef _kernel_arch_x86_c
#define _kernel_arch_x86_c
#include <arch/x86/x86.h>

int init_arch( void ){
	init_tables(); printf( "[\x12+\x17] initialised tables\n" );
	init_paging(); printf( "[\x12+\x17] initialised paging\n" );
	init_timer(TIMER_FREQ);	printf( "[\x12+\x17] Initialised timer to %uhz\n", TIMER_FREQ );

	/*
	init_console();
	init_keyboard(); 	//printf( "[\x12+\x17] initialised keyboard\n" );
	init_ide( 0x1F0, 0x3F4, 0x170, 0x374, 0x000 );
	*/

	return 0;
}

unsigned char inb( unsigned short port ){
        unsigned char ret;
        asm volatile( "inb %1, %0" : "=a"(ret) : "Nd"(port) );
        return ret;
}

void outb( unsigned short _port, unsigned char _data ){
	asm volatile( "outb %1, %0" : : "dN" (_port), "a" (_data));
}	

void outl( unsigned short _port, unsigned long _data ){
	asm volatile( "outl %1, %0" : : "dN" (_port), "a" (_data));
}	

void reboot(){
	asm volatile( "cli" );
	unsigned char good = 2;
	while ( good & 2 )
		good = inb( 0x64 );
	outb( 0x64, 0xfe );
	asm volatile ( "hlt" );
}

#endif
