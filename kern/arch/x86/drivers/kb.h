#ifndef _kernel_keyboard_h
#define _kernel_keyboard_h
#include <arch/x86/isr.h>
#include <sys/skio.h>
#include <fs/fs.h>
#include <fs/devfs.h>

#define KB_LA 0xf0
#define KB_RA 0xf1
#define KB_UA 0xf2
#define KB_DA 0xf3

void init_keyboard( void );

#endif