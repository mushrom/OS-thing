#ifndef _kernel_arch_x86_gdt_c
#define _kernel_arch_x86_gdt_c
#include <stdint.h>
#include <arch/x86/gdt.h>

gdt_entry_t gdt_entries[5];
gdt_ptr_t   gdt_ptr;

static void init_gdt( void ){
	gdt_ptr.limit = (sizeof(gdt_entry_t) * 5 ) - 1;
	gdt_ptr.base  = (uint32_t)&gdt_entries;

	gdt_set_gate( 0, 0, 0, 		0,    0    );
	gdt_set_gate( 1, 0, 0xffffffff, 0x9a, 0xcf );
	gdt_set_gate( 2, 0, 0xffffffff, 0x92, 0xcf );
	gdt_set_gate( 3, 0, 0xffffffff, 0xfa, 0xcf );
	gdt_set_gate( 4, 0, 0xffffffff, 0xf2, 0xcf );

	gdt_flush(( uint32_t)&gdt_ptr );
}

static void gdt_set_gate( int32_t num, uint32_t base, uint32_t limit, uint8_t access, uint8_t gran ){
	gdt_entries[num].base_low	= ( base & 0xffff );
	gdt_entries[num].base_middle	= ( base >> 16 ) & 0xff;
	gdt_entries[num].base_high	= ( base >> 24 ) & 0xff;

	gdt_entries[num].limit_low	= (limit & 0xffff);
	gdt_entries[num].granularity	= (limit >> 16 ) & 0x0f;

	gdt_entries[num].granularity	|= gran & 0xf0;
	gdt_entries[num].access		= access;
}

#endif
