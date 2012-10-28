#ifndef _kernel_multiboot_h
#define _kernel_multiboot_h

enum {
	MULTIBOOT_FLAG_MEM	= 0x001,
	MULTIBOOT_FLAG_DEVICE	= 0x002,
	MULTIBOOT_FLAG_CMDLINE	= 0x004,
	MULTIBOOT_FLAG_MODS	= 0x008,
	MULTIBOOT_FLAG_AOUT	= 0x010,
	MULTIBOOT_FLAG_ELF	= 0x020,
	MULTIBOOT_FLAG_MMAP	= 0x040,
	MULTIBOOT_FLAG_CONFIG	= 0x080,
	MULTIBOOT_FLAG_LOADER	= 0x100,
	MULTIBOOT_FLAG_APM	= 0x200,
	MULTIBOOT_FLAG_VBE	= 0x400
};

typedef struct multiboot_header {
	unsigned long flags;
	unsigned long mem_lower;
	unsigned long mem_upper;
	unsigned long boot_device;
	unsigned long cmdline;
	unsigned long mods_count;
	unsigned long mods_addr;
	
	unsigned long num;
	unsigned long size;
	unsigned long addr;
	unsigned long shndx;
	unsigned long mmap_length;
	unsigned long mmap_addr;
	unsigned long drives_length;
	unsigned long drives_addr;
	unsigned long config_table;
	unsigned long boot_loader_name;
	unsigned long apm_table;
	unsigned long vbe_control_info;
	unsigned long vbe_mode;
	unsigned long vbe_interface_seg;
	unsigned long vbe_interface_off;
	unsigned long vbe_interface_len;
} __attribute__((packed)) multiboot_header_t;
	
#endif
