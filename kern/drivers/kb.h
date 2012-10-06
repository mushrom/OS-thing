#ifndef _kernel_keyboard_h
#define _kernel_keyboard_h
#include <arch/x86/isr.h>
#include <sys/skio.h>
#include <fs/fs.h>
#include <fs/devfs.h>

void init_keyboard( void );

#endif
