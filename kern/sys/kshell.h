#ifndef _kernel_shell_h
#define _kernel_shell_h

#include <sys/console.h>
#include <sys/skio.h>
#include <sys/syscall.h>

#include <lib/kmacros.h>
#include <lib/string.h>
#include <lib/stdio.h>

#include <arch/x86/timer.h>
#include <arch/x86/isr.h>
#include <arch/x86/kheap.h>
#include <arch/x86/init_tables.h>

#include <drivers/driver.h>

void kshell( char * );

#endif
