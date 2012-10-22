#ifndef _kernel_shell_h
#define _kernel_shell_h

//#include <sys/console.h>
#include <sys/skio.h>

#include <lib/kmacros.h>
#include <lib/string.h>
#include <lib/stdio.h>

#include <arch/arch.h>

#include <mem/alloc.h>

#include <fs/fs.h>

typedef int (*shell_func_t)( int argc, char **argv );
typedef struct shell_command {
	char *name;
	char *help;
	shell_func_t function;
} shell_cmd_t;

int  kshell( int, char ** );
void register_shell_func( char *name, shell_func_t function );
void init_shell( );

#endif
