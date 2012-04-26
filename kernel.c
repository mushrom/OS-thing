/* ======================================================\
 * Mah kernel, v0x00000001
 * Simple machine
 * Props to wiki.osdev.org. ^_^
 *======================================================*/

#include "sys/skio.h"
#include "sys/console.h"
//Main kernel code
void kmain( void* mbd, unsigned int magic ){
	unsigned int i = 0;
	unsigned short int j = i;
	unsigned char *khello_world = "Welcome to mah os! :D\n"
				      "Obsidian kernel v0x00000001 shell\n\nobsidian > ";
	color = 0x03;

	cls();
	kputs( khello_world );

	kputs(  "\nTesting a slightly longer string, to see how this goes exactly.\n"
		"Long string.\n\n"
		"This string is intended to test teh scrolling function.\n\n"
		"Let's see how it goes.\n"
		"obsidian > ls \n"
		"       something    aFile.txt   meh.c \n\n" 
		"scroll\n"
		"scrolling\n"
		"random crap\n\n"
		"So how's it scrollin'\n"
		"Still not pushing teh limits, must go further\n\n"
		"Is is broken yet?\n\n"
		"Still need lines!\n"
		"Shouldn't be much longer now\n"
		"I believe this is where it will break\n\n\n\n"
		"Oh cool, it works. ^_^\n"
	);
	while ( 1 ){
		kputchar((j%26)+'a');
		for ( i = 0; i < 6000000; i++ );
		j++;
	}
	
}
