extern main

global _start
_start:
	call main
	mov eax, 1
	int 0x50
