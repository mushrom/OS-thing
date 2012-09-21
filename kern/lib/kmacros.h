#ifndef _kernel_debug_macros
#define _kernel_debug_macros

#define PANIC( a ) { printf( "Great, you broke it. Sit here and think about what you did."\
				"\n\t%s\n\t%s:%d\n", a, __FILE__, __LINE__ );\
				asm volatile( "hlt" );\
		   }

				//asm volatile ( "sti" );
#ifndef NODBG
#define assert( a, b ) { if ( !(a) ){ printf( "Assertion at %s:%d failed\n", __FILE__, __LINE__ ); return b; }}
#else
#define assert( a, b ) /*a b*/
#endif

#endif
