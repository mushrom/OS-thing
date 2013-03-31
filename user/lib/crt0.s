; C runtime 0 for Obsidian userland
extern crt1_main

global _start
_start:
	mov eax, [esp]
	mov ebx, [esp + 4]
	mov ecx, [esp + 8]
	push ecx
	push ebx
	push eax
	call crt1_main
	add esp, 12 
	push eax
	mov eax, 1
	int 0x50
