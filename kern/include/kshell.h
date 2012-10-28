#ifndef _kernel_shell_h
#define _kernel_shell_h

#include <skio.h>

#include <kmacros.h>
#include <string.h>
#include <stdio.h>

#include <alloc.h>

#include <fs.h>
#include <ipc.h>
#include <task.h>
#include <timer.h>
#include <syscall.h>

typedef int (*shell_func_t)( int argc, char **argv );
typedef struct shell_command {
	char *name;
	char *help;
	shell_func_t function;
} shell_cmd_t;

void kshell( );
void register_shell_func( char *name, shell_func_t function );
void init_shell( );

#endif
