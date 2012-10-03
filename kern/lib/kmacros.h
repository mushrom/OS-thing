#ifndef _kernel_debug_macros
#define _kernel_debug_macros
extern char *g_errfile;
extern unsigned int g_errline;

#ifdef NO_DEBUG
#define PANIC( a ) { printf( "Great, you broke it. Sit here and think about what you did."\
				"\n\t%s\n\t%s:%d\n", a, __FILE__, __LINE__ );\
				asm volatile( "hlt" );\
		   }
#else
#define PANIC( a ) { printf( "Great, you broke it. Sit here and think about what you did."\
				"\n\t%s\n\t%s:%d\n", a, g_errfile, g_errline );\
				asm volatile( "hlt" );\
		   }
#endif

#ifndef NO_DEBUG
#define assert( a, b ) { if ( !(a) ){ printf( "Assertion at %s:%d failed\n", __FILE__, __LINE__ ); return b; }}
#define DEBUG_HERE {g_errfile = __FILE__; g_errline = __LINE__; };
#else
#define assert( a, b ) /*a b*/
#define DEBUG_HERE /* Remove -DNO_DEBUG flag to enable debugging */
#endif


#endif
