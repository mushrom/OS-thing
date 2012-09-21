#ifndef _kernel_arch_x86_h
#define _kernel_arch_x86_h
#include <sys/console.h>
#include <arch/x86/init_tables.h>
#include <arch/x86/kheap.h>
#include <arch/x86/paging.h>
#include <arch/x86/timer.h>
#include <arch/x86/task.h>

int arch_init( void );

#endif
