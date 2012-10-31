#ifndef _kernel_elf_c
#define _kernel_elf_c
#include <elf.h>

char elf_h_magic[4] = { 0x7f, 'E', 'L', 'F' };

int load_elf( int fd ){
	extern task_t *current_task;
	printf( "Checking file...\n" );
	if ((unsigned)fd > MAX_FILES || !current_task->files[fd] || fd < 0 )
		return -1;

	char *buf = (char *)kmalloc( sizeof( Elf32_Ehdr ), 0, 0 );
	int ret = read( fd, buf, sizeof( Elf32_Ehdr ));
	Elf32_Ehdr *elf_header = (void *)buf;
	int i;

	if ( ret < sizeof( Elf32_Ehdr ))
		return -1;

	for ( i = 0; i < 4 && i < ret; i++ ){
		if ( buf[i] != elf_h_magic[i] )
			return -1;
	}

	if ( elf_header->e_ident[EI_CLASS] == ELFCLASS32 )
		printf( "[debug] Is 32-bit executable\n" );

	if ( elf_header->e_ident[EI_DATA]  == ELFDATA2LSB )
		printf( "[debug] Is little-endian\n" );

	map_pages( 0x8000000, 0x8050000, 7, current_task->dir );
	//set_page_dir( current_task->dir );
	flush_tlb();
	memset((void *)0x8001000, 0, 4 );
	
	printf( "[debug] entry: 0x%x\tsections: 0x%x\n", elf_header->e_entry, elf_header->e_shnum );

	return 0;
}

#endif
