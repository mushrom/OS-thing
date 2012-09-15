#ifndef _kernel_arch_x86_h
#define _kernel_arch_x86_h
#include <sys/skio.h>
#include <arch/x86/gdt.c>

int arch_init( void ){
	kputs( "inited gdt" );
	kputs( "This is x86 arch\n" );
	init_gdt();
	return 0;
}

#endif
