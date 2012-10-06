#ifndef _kernel_console_h
#define _kernel_console_h
#include <lib/string.h>
#include <drivers/driver.h>

#define XSIZE 80 /*Actual terminal size is 80x25, this is 160 because the videoram needs 16 bit input*/
#define YSIZE 25

#define VIDEORAM 0xb8000;
#define COLOR_BLACK  0x00
#define COLOR_BLU    0x01
#define COLOR_GREEN  0x02
#define COLOR_CYAN   0x03
#define COLOR_RED    0x04
#define COLOR_PURPLE 0x05
#define COLOR_BROWN  0x06
#define COLOR_LGRAY  0x07
#define COLOR_DGRAY  0x08
#define COLOR_LBLUE  0x09
#define COLOR_LGREEN 0x0a
#define COLOR_LCYAN  0x0b
#define COLOR_LRED   0x0c
#define COLOR_LPURPLE 0xd
#define COLOR_YELLOW 0x0e
#define COLOR_WHITE  0x0f

void cls( void );
//void _kcheck_scroll( void );
void kputchar( unsigned char );
void kputs( char * );
void set_color( unsigned char );
void init_console();

#endif
