#ifndef _kernel_arch_x86_c
#define _kernel_arch_x86_c
#include <arch/x86/x86.h>

int arch_init( void ){
	init_tables();
	kputs( "[\x12+\x17] initialised tables\n" );

	//init_paging();
	kputs( "[\x12+\x17] initialised paging\n" );

	return 0;
}

#endif
