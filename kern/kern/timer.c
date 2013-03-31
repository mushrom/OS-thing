#ifndef _kernel_timer_c
#define _kernel_timer_c
#include <timer.h>

uint32_t tick = 0;
uint32_t current_freq = 0;
unsigned long uptime = 0;
unsigned long cpu_speed = 0;

static void timer_call( registers_t *regs ){
	tick++;
	switch_task();
}

void init_timer( uint32_t freq ){
	register_interrupt_handler( IRQ0, &timer_call );
	current_freq = freq;

	uint32_t divisor = PIT_FREQ / freq;
	uint8_t  l = (uint8_t)( divisor & 0xff );
	uint8_t  h = (uint8_t)( divisor >> 8 ) & 0xff;

	outb( 0x43, 0x36 );
	outb( 0x40, l );
	outb( 0x40, h );
}

unsigned long get_uptime( void ){
	return tick/current_freq;
}

unsigned long get_tick( void ){
	return tick;
}

void sleep( uint32_t seconds ){
	uint32_t limit = seconds * current_freq;
	unsigned long mark;
	mark = tick;
	while ( tick - mark < limit );
}

void usleep( uint32_t useconds ){
	uint32_t limit = useconds * current_freq / 1000;
	unsigned long mark;
	mark = tick;
	while ( tick - mark < limit );
}

#endif
