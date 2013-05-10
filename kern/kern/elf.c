#ifndef _kernel_elf_c
#define _kernel_elf_c
#include <elf.h>

char elf_h_magic[4] = { 0x7f, 'E', 'L', 'F' };

/* \brief Load a static ELF executable from an open file descriptor
 * @param fd The file descript to read from
 * @return Nothing if successful, -1 otherwise.
 */
int load_elf( int fd, char **argv, char **envp ){
	extern task_t *current_task;
	//printf( "Checking file...\n" );
	if ((unsigned)fd > MAX_FILES || !current_task->files[fd] || fd < 0 )
		return -1;

	Elf32_Ehdr elf_header;
	Elf32_Phdr phbuf;
	//Elf32_Shdr shbuf;
	int ret = read( fd, &elf_header, sizeof( Elf32_Ehdr ));
	char *buf = (char *)&elf_header;
	int i;

	if ( ret < sizeof( Elf32_Ehdr ))
		return -1;

	for ( i = 0; i < 4 && i < ret; i++ ){
		if ( buf[i] != elf_h_magic[i] )
			return -1;
	}

	if ( elf_header.e_ident[EI_CLASS] != ELFCLASS32 )
		return -1;
		//printf( "[debug] Is 32-bit executable\n" );
	//else return -1;
	

	if ( elf_header.e_ident[EI_DATA]  != ELFDATA2LSB )
		return -1;
		//printf( "[debug] Is little-endian\n" );
	//else return -1;
	
	//printf( "[debug] entry: 0x%x\tsections: 0x%x\n", elf_header.e_entry, elf_header.e_shnum );
	//printf( "[debuf] program header offset: 0x%x, %d pheads, size: %d\n", 
	//		elf_header.e_phoff, elf_header.e_phnum, elf_header.e_phentsize );
	page_dir_t *new_dir = clone_page_dir( current_task->dir );
	set_page_dir( new_dir );

	for ( i = 0; i < elf_header.e_phnum; i++ ){
		lseek( fd, elf_header.e_phoff + ( i * elf_header.e_phentsize ), 0 );
		read( fd, &phbuf, elf_header.e_phentsize );
	/*
	printf( "[debug] Phead type: 0x%x, vaddr: 0x%x, offset: 0x%x, rsize: 0x%x, vsize: 0x%x\n", 
		phbuf.p_type, phbuf.p_vaddr, phbuf.p_offset, phbuf.p_filesz, phbuf.p_memsz );
	*/

		buf = (char *)kmalloc( phbuf.p_filesz, 0, 0 );
		lseek( fd, phbuf.p_offset, 0 );
		read( fd, buf, phbuf.p_filesz );

		map_pages( phbuf.p_vaddr - PAGE_SIZE * 2, phbuf.p_vaddr + phbuf.p_memsz, 7, new_dir );
		memcpy((void *)phbuf.p_vaddr, buf, phbuf.p_filesz );
		//kfree( buf );
	}

	create_process((void *)elf_header.e_entry, argv, envp, phbuf.p_vaddr - PAGE_SIZE * 2, phbuf.p_vaddr + phbuf.p_memsz );
	
	//free_pages( 0x8000000, 0x8050000, current_task->dir );
	//flush_tlb();
	return 0;
}

#endif
