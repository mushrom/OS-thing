#ifndef _kernel_fat_c
#define _kernel_fat_c
#include <fat.h>

void test_fatfs( char *path, unsigned long offset ){
	fat_bs_t 	bs;
	int fd, ret; 
	fd = syscall_open( path, 0 );

	if ( fd < 0 ){
		printf( "    [fat] Could not open path\n" );
		return;
	}

	lseek( fd, offset, 0 );
	ret = syscall_read(  fd, &bs,    sizeof( fat_bs_t ));

	if ( ret < sizeof( fat_bs_t )){
		printf( "    [fat] Could not read fat boot sector\n" );
		return;
	}

	unsigned long root_dir_sectors 	= ((bs.root_entry_count * 32 ) + 
		(bs.bytes_per_sector - 1 ) / bs.bytes_per_sector );
	unsigned long first_data_sector = bs.reserved_sector_count + (bs.table_count * bs.table_size_16 );
	unsigned long data_sectors 	= bs.total_sectors_16 - ( bs.reserved_sector_count + bs.table_count *
		bs.table_size_16 ) + root_dir_sectors;
	unsigned long total_clusters 	= data_sectors / bs.sectors_per_cluster;

	unsigned long fat_type;
	if ( total_clusters < 4085 )
		fat_type = 12;
	else if ( total_clusters < 65525 )
		fat_type = 16;
	else
		fat_type = 32;

	unsigned long root_cluster;
	if ( fat_type < 32 )
		root_cluster = first_data_sector;
	else
		root_cluster = ((fat_extbs_32_t *)(&bs.extended_section))->root_cluster;

	printf( "Root dir sectors: %d, bytes per sector: %d ", root_dir_sectors, bs.bytes_per_sector );
	printf( "(%dkb)\n", ( root_dir_sectors * bs.bytes_per_sector ) / 1024 );
	printf( "First data sector: %d\n", first_data_sector );
	printf( "Total clusters: %d\n", total_clusters );
	printf( "Is fat%d fs\n", fat_type );
	printf( "root cluster: %d\n", root_cluster );

	syscall_close( fd );

}

#endif
