#ifndef _user_fat_h
#define _user_fat_h
#include <syscall.h>
//#include <stdio.h>

typedef struct fat_extbs_32 { 
	unsigned int 	table_size;
	unsigned short 	extended_flags;
	unsigned short 	fat_version;
	unsigned int 	root_cluster;
	unsigned short 	fat_info;
	unsigned short	backup_bs_sector;
	unsigned char 	reserved_0[12];
	unsigned char 	drive_number;
	unsigned char 	reserved_1;
	unsigned char 	boot_sig;
	unsigned int 	volume_id;
	unsigned char	volume_label[11];
	unsigned char 	fat_type_label[8];
} __attribute__((packed)) fat_extbs_32_t;

typedef struct fat_extbs_16 {
	unsigned char	bios_drive_num;
	unsigned char	reserved1;
	unsigned char	boot_signature;
	unsigned int	volume_id;
	unsigned char	volume_label[11];
	unsigned char	fat_type_label[8];
} fat_extbs_16_t;

typedef struct fat_bs {
	unsigned char	bootjmp[3];
	unsigned char	oem_name[8];
	unsigned short	bytes_per_sector;
	unsigned char	sectors_per_cluster;
	unsigned short	reserved_sector_count;
	unsigned char	table_count;
	unsigned short	root_entry_count;
	unsigned short 	total_sectors_16;
	unsigned char	media_type;
	unsigned short	table_size_16;
	unsigned short	sectors_per_track;
	unsigned short	head_size_count;
	unsigned int	hidden_sector_count;
	unsigned int 	total_sectors_32;

	unsigned char	extended_section[54];
} __attribute__((packed)) fat_bs_t;

void test_fatfs( char *path, unsigned long offset );

#endif
