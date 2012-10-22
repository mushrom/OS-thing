.global _loader
.extern _kmain

interrupt_v_tables:
	b . @ reset
	b . 
	b . @ swi
	b .
	b .
	b .
	b .
	b .

.comm stack, 0x10000
_loader:
	ldr sp, =stack+0x10000
	bl kmain
end:
	b end
