#ifndef _kernel_arch_x86_gdt_h
#define _kernel_arch_x86_gdt_h
#include <stdint.h>

struct gdt_entry {
	uint16_t limit_low;
	uint16_t base_low;
	uint8_t  base_middle;
	uint8_t  access;
	uint8_t  granularity;
	uint8_t  base_high;
} __attribute__((packed));
typedef struct gdt_entry gdt_entry_t;

typedef struct gdt_ptr {
	uint16_t limit;
	uint32_t base;
} gdt_ptr_t;

static void init_gdt( void );
static void gdt_set_gate( int32_t, uint32_t, uint32_t, uint8_t, uint8_t );
static void gdt_flush( uint32_t );

#endif
