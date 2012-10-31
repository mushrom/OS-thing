#ifndef _kernel_timer_h
#define _kernel_timer_h
#include <stdint.h>
#include <stdio.h>
#include <isr.h>
#include <task.h>

#define PIT_FREQ 1193180
#define TIMER_FREQ 1000

void init_timer( uint32_t );
void wait( uint32_t );
void usleep( uint32_t );
unsigned long get_uptime( void );
unsigned long get_tick( void );

#endif
