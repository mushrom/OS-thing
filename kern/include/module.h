#ifndef _kernel_module_h
#define _kernel_module_h
#include <task.h>
#include <fs.h>

int load_module( char *path, int flags );

#endif
