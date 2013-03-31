#ifndef _user_fat_c
#define _user_fat_c
#include <stdio.h>
#include <fat.h>

int main(){
	printf( "testing:\t 0x%x\n", 0xdeadbeef );
	printf( "main:\t\t 0x%x\n", main );
	return 0;
}

#endif
