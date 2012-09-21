#ifndef _kernel_console_h
#define _kernel_console_h
#include <lib/string.h>

#define XSIZE 160 /*Actual terminal size is 80x25, this is 160 because the videoram needs 16 bit input*/
#define YSIZE 25

#define VIDEORAM 0xb8000;
#define COLOR_BLACK  0x00
#define COLOR_RED    0x01
#define COLOR_GREEN  0x02
#define COLOR_YELLOW 0x03
#define COLOR_BLUE   0x04
#define COLOR_PURPLE 0x05
#define COLOR_CYAN   0x06
#define COLOR_GRAY   0x07

void cls( void );
void _kcheck_scroll( void );
void kputchar( unsigned char );
void kputs( char * );
void set_color( unsigned char );

#endif
