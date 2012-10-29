#ifndef _kernel_elf_c
#define _kernel_elf_c
#include <elf.h>

char elf_h_magic[4] = { 0x7f, 'E', 'L', 'F' };

int load_elf( int fd ){
	extern task_t *current_task;
	if ( fd > MAX_FILES || !current_task->files[fd] )
		return -1;

	file_node_t *fp = current_task->files[fd]->file;
	if ( fp->size < sizeof( Elf32_Ehdr ))
		return -1;

	char *buf = (char *)kmalloc( fp->size, 0, 0 );
	int ret = fs_pread( fp, buf, fp->size, 0 );
	Elf32_Ehdr *elf_header = buf;
	int i;

	if ( ret < sizeof( Elf32_Ehdr ))
		return -1;

	for ( i = 0; i < 4 && i < ret; i++ ){
		if ( buf[i] != elf_h_magic[i] )
			return -1;
	}
	printf( "[debug] is elf executable\n" );

	if ( elf_header->e_ident[EI_CLASS] == ELFCLASS32 )
		printf( "[debug] Is 32-bit executable\n" );

	if ( elf_header->e_ident[EI_DATA]  == ELFDATA2LSB )
		printf( "[debug] Is little-endian\n" );
	
	printf( "[debug] entry: 0x%x\tsections: 0x%x\n", elf_header->e_entry, elf_header->e_shnum );

	return 0;
}

#endif
