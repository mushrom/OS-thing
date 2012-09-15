/* =====================================================*\
 * Mah kernel, v0x00000001
 * Simple machine
 * Props to wiki.osdev.org. ^_^
\*======================================================*/

#include <sys/skio.h>
#include <sys/console.h>
/*#include "sys/con_new.h"*/
#include <sys/mem.h>
#include <lib/itoa.h>
#include <arch/arch.h>
#include <lib/stdint.h>

/* Main kernel code */
void kmain( void* mbd, unsigned int magic ){
	int i = 0;
	set_color( COLOR_GREEN );
	cls();

	arch_init();
	init_kheap((void *)KMEMROOT, 0x100000 );

	char 	*khello_world = "Welcome to mah os man! :D\n",
		*testalloc = (char *)kmalloc( strlen( khello_world ));

	memcpy(( void *)testalloc, ( void *)khello_world, strlen( khello_world ));
	kputs( testalloc );

	kputs( "Allocated mem: " ); print_num( get_memused( )); kputs( " bytes\n" );

	for ( i = 0; i < 4; i += 1 ){
		kputs( "Allocated mem: 0x" );
		print_hex( get_memused( ));
		kputs( " bytes \t" );

		kputs( "testalloc at: 0x" );
		print_hex(( unsigned int )testalloc );
		kputs( "\n" );
		if (( testalloc = kmalloc( 0x1000 )) == 0 ){
			kputs( "\nOut of memory\n" );
			break;
		}
		memcpy(( void *)testalloc, ( void *)khello_world, strlen( khello_world ));
	}

	//kfree( testalloc );

}
