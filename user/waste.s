global _start

msg: db "Hello, userworld. ^_^",0xa,0x0

_start:
	push 0
	mov ebx, msg
	mov eax, 10
	int 0x50
	
;.alabel:
	;jmp .alabel
	mov eax, 1
	push eax
	int 0x50
