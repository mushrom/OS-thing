extern main

global _start
_start:
	mov eax, [esp + 4]
	mov ebx, [esp + 8]
	push ebx
	push eax
	call main
	add esp, 8
	push eax
	mov eax, 1
	int 0x50
