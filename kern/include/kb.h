#ifndef _kernel_keyboard_h
#define _kernel_keyboard_h
#include <isr.h>
#include <skio.h>
#include <fs.h>
#include <devfs.h>

#define KB_LA 0xf0
#define KB_RA 0xf1
#define KB_UA 0xf2
#define KB_DA 0xf3

file_node_t *keyboard_create( );

#endif
