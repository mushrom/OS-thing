#ifndef _kernel_arch_x86_h
#define _kernel_arch_x86_h
#include <arch/x86/init_tables.h>
#include <arch/x86/timer.h>
#include <arch/x86/drivers/console.h>
#include <arch/x86/drivers/kb.h>
#include <arch/x86/drivers/ide.h>
#include <arch/x86/task.h>

#define insl( port, buffer, count )\
	asm volatile ( "cld; rep; insl" :: "D"(buffer), "d"(port), "c"(count))

int init_arch( void );

unsigned char inb( unsigned short );
void outb( unsigned short, unsigned char );
void outl( unsigned short, unsigned long );
void reboot();

#endif
