#ifndef _kernel_timer_h
#define _kernel_timer_h
#include <lib/stdint.h>
#include <lib/stdio.h>
#include <arch/x86/isr.h>
#include <arch/x86/task.h>

#define PIT_FREQ 1193180
#define TIMER_FREQ 1000

void init_timer( uint32_t );
void wait( uint32_t );
void usleep( uint32_t );

#endif
