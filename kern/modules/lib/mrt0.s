global _start
extern module_init

_start:
	call module_init
	push eax
	mov eax, 1
	int 0x50
