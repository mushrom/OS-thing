global _start

_start:
	xor eax, eax
	int 0x50
	mov eax, 1
	push eax
	int 0x50
