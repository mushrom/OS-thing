#ifndef _kernel_ide_c
#define _kernel_ide_c
#include "ide.h"
/* This source is from the wiki over at wiki.osdev.org */

typedef struct ide_channel_regs {
	unsigned short base;
	unsigned short ctrl;
	unsigned short bmide;
	unsigned char  nien;
} ide_channel_regs_t;

typedef struct ide_device {
	unsigned char  reserved;
	unsigned char  channel;
	unsigned char  drive;
	unsigned short type;
	unsigned short signature;
	unsigned short capabilities;
	unsigned int   command_sets;
	unsigned int   size;
	unsigned char  model[41];
} ide_device_t;
	
ide_channel_regs_t 	channels[2]; ide_device_t		ide_devices[4];
unsigned char ide_buf[2048] = {0};
unsigned char ide_irq_invoked = 0;
unsigned char atapi_packet[12] = { 0xa8, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };
unsigned char package[3] = {0};
int ide_driver_read( file_node_t *node, void *buf, unsigned long size );
int ide_driver_write( file_node_t *node, void *buf, unsigned long size );
int ide_driver_pread( file_node_t *node, void *buf, unsigned long size, unsigned long offset );
int ide_driver_pwrite( file_node_t *node, void *buf, unsigned long size, unsigned long offset );

void init_ide( unsigned int bar0, unsigned int bar1, unsigned int bar2, unsigned int bar3, unsigned int bar4 ){
	int i, j, k, count = 0;

	channels[ATA_PRIMARY   ].base	= (bar0 & 0xfffffffc) + 0x1f0 * (!bar0);
	channels[ATA_PRIMARY   ].ctrl	= (bar1 & 0xfffffffc) + 0x3f4 * (!bar1);
	channels[ATA_SECONDARY ].base	= (bar2 & 0xfffffffc) + 0x170 * (!bar2);
	channels[ATA_SECONDARY ].ctrl	= (bar3 & 0xfffffffc) + 0x374 * (!bar3);
	channels[ATA_PRIMARY   ].bmide	= (bar4 & 0xfffffffc) + 0;
	channels[ATA_SECONDARY ].bmide	= (bar4 & 0xfffffffc) + 8;

	ide_write( ATA_PRIMARY, ATA_REG_CONTROL, 2 );
	ide_write( ATA_SECONDARY, ATA_REG_CONTROL, 2 );

	for ( i = 0; i < 2; i++ ){
		for ( j = 0; j < 2; j++ ){
			unsigned char err = 0, type = IDE_ATA, status;
			ide_devices[count].reserved = 0;

			ide_write( i, ATA_REG_HDDEVSEL, 0xa0 | (j << 4));
			usleep(1);

			ide_write( i, ATA_REG_COMMAND, ATA_CMD_IDENTIFY );
			usleep(1);

			if ( ide_read( i, ATA_REG_STATUS ) == 0 ) continue;

			while ( 1 ){
				status = ide_read( i, ATA_REG_STATUS );
				if ((status & ATA_SR_ERR)){ err = 1; break; }
				if (!(status & ATA_SR_BSY) && (status & ATA_SR_DRQ)) break;
			}

			if ( err != 0 ){
				unsigned char cl = ide_read( i, ATA_REG_LBA1 );
				unsigned char ch = ide_read( i, ATA_REG_LBA2 );

				if ( cl == 0x14 && ch == 0xeb )
					type = IDE_ATAPI;
				else if ( cl == 0x69 && ch == 0x96 )
					type = IDE_ATAPI;
				else 
					continue;

				ide_write( i, ATA_REG_COMMAND, ATA_CMD_IDENTIFY_PACKET );
				usleep( 1 );
			}

			ide_read_buffer( i, ATA_REG_DATA, (unsigned int)ide_buf, 128 );

			ide_devices[count].reserved	= 1;
			ide_devices[count].type		= type;
			ide_devices[count].channel	= i;
			ide_devices[count].drive	= j;
			ide_devices[count].signature	= *((unsigned short *)(ide_buf + ATA_IDENT_DEVICETYPE ));
			ide_devices[count].capabilities	= *((unsigned short *)(ide_buf + ATA_IDENT_CAPABILITIES ));
			ide_devices[count].command_sets	= *((unsigned int *)(ide_buf + ATA_IDENT_COMMANDSETS ));

			if ( ide_devices[count].command_sets & ( 1 << 26 ))
				ide_devices[count].size = *((unsigned int *)(ide_buf + ATA_IDENT_MAX_LBA_EXT ));
			else
				ide_devices[count].size = *((unsigned int *)(ide_buf + ATA_IDENT_MAX_LBA ));

			for ( k = 0; k < 40; k += 2 ){
				ide_devices[count].model[k] = ide_buf[ ATA_IDENT_MODEL + k + 1 ];
				ide_devices[count].model[k+1] = ide_buf[ ATA_IDENT_MODEL + k ];
			}
			ide_devices[count].model[40] = 0;

			count++;
		}
	}
	register_interrupt_handler( IRQ14, ide_irq_handler );
	register_interrupt_handler( IRQ15, ide_irq_handler );
	register_interrupt_handler( IRQ11, ide_irq_handler );
	register_interrupt_handler( IRQ9,  ide_irq_handler );

	file_node_t ide_file;
	char *dev_name = "ide0";
	for ( i = 0; i < 4; i++ ){
		if ( ide_devices[i].reserved ){
			printf( "    %s: %s drive, %dkb (%d): %s\n", 
				dev_name,
				(char *[]){"ATA", "ATAPI"}[ide_devices[i].type],
				(ide_devices[i].size + 1)/ 2,
				ide_devices[i].size, 
				ide_devices[i].model
			);

			memset( &ide_file, 0, sizeof( file_node_t ));
			memcpy( ide_file.name, dev_name, 4 );
			ide_file.type	= FS_BLOCK_D;
			ide_file.read	= ide_driver_read;
			ide_file.write	= ide_driver_write;
			ide_file.pread	= ide_driver_pread;
			ide_file.pwrite	= ide_driver_pwrite;
			ide_file.dev_id	= i;
			devfs_register_device( ide_file );
			dev_name[3]++;
		}
	}

	char *part_table = (char *)kmalloc( 512, 0, 0 );
	memset( part_table, 0, 512 );
	/* "i < 2" for testing only, switch 2 to 4 once atapi is finished */
	for ( i = 0; i < 4; i++ ){
		if ( ide_devices[i].reserved ){
			ide_read_sectors( i, 1, 0, 0, (unsigned long)part_table );
			for ( j = 0x1be; j <= 0x1ee; j += 0x10 ){
				if ( part_table[j] ){
					printf( "    ide%d has partition at 0x%x\n", i, j );
					printf( "          id: 0x%x\n", (int)part_table[j+5]);
				} 
			}
			memset( part_table, 0, 512 );
		}
	}
}

int ide_driver_read( file_node_t *node, void *buf, unsigned long size ){
	char read_buf[512];
	char *in_buf = buf;
	unsigned long sectors = ( size / 512 ) + (( size % 512 )?1:0);
	unsigned int  i = 0, j = 0, l = 0, k = 0, device = node->dev_id;
	int ret = 0;

	//printf( "[debug] reading %u sectors\n", sectors );

	for ( i = 0; i < sectors && !ret; i++ ){
		//printf( "[debug] Got sector %d\n", i );
		ret = ide_read_sectors( device, 1, i, 0, (unsigned int)read_buf );
		if ( !ret ){
			for ( k = j, l = 0; j < k + 512 && j < size; j++, l++ ){
				in_buf[j] = read_buf[l];
			}
		} 
		memset( &read_buf, 0, 512 );
	}

	return j;
}

int ide_driver_write( file_node_t *node, void *buf, unsigned long size ){ DEBUG_HERE
	char write_buf[512];
	char *in_buf = buf;
	unsigned long sectors = ( size / 512 ) + (( size % 512 )?1:0);
	unsigned int  i = 0, j = 0, l = 0, k = 0, device = node->dev_id;
	int ret = 0;

	//printf( "[debug] writing %u sectors\n", sectors );

	for ( i = 0; i < sectors && !ret; i++ ){ DEBUG_HERE
		ide_driver_pread( node, write_buf, 512, i * 512 );
		for ( l = 0, k = j; j < k + 512 && j < size; l++, j++ ){ DEBUG_HERE
			write_buf[l] = in_buf[j];
		 }
		DEBUG_HERE;
		ret = ide_write_sectors( device, 1, i, 0, (unsigned int)write_buf );
	 DEBUG_HERE }

	return j;
 DEBUG_HERE }

int ide_driver_pread( file_node_t *node, void *buf, unsigned long size, unsigned long offset ){
	char read_buf[512];
	char *in_buf = buf;
	unsigned long sectors = ( size / 512 ) + (( size % 512 )?1:0);
	unsigned long soffset = offset / 512;
	unsigned int  i = 0, j = 0, l = 0, k = 0, device = node->dev_id;
	int ret = 0;

	printf( "[debug] reading %u sectors\n", sectors );

	for ( i = soffset; i < sectors + soffset + ( offset % 512 > 0 ) && !ret; i++ ){
		printf( "[debug] Got sector %d\n", i );
		ret = ide_read_sectors( device, 1, i, 0, (unsigned int)read_buf );
		if ( !ret ){
			if ( i == soffset )
				l = offset % 512;
			else 
				l = 0;
			for ( k = j; j < k + 512 && ( !l || l % 512 ) && j < size; j++, l++ ){
				in_buf[j] = read_buf[l];
			}
		} 
		memset( &read_buf, 0, 512 );
	}

	return j;
}

int ide_driver_pwrite( file_node_t *node, void *buf, unsigned long size, unsigned long offset ){
	char write_buf[512];
	char *in_buf = buf;
	unsigned long sectors = ( size / 512 ) + (( size % 512 )?1:0);
	unsigned long soffset = offset / 512;
	unsigned int  i = 0, j = 0, l = 0, k = 0, device = node->dev_id;
	int ret = 0;

	printf( "[debug] writing %u sectors\n", sectors );

	for ( i = soffset; i < sectors + soffset + ( offset % 512 > 0 ) && !ret; i++ ){
		printf( "[debug] Got sector %d\n", i );
		ide_driver_pread( node, write_buf, 512, i * 512 );
		if ( i == soffset )
			l = offset % 512;
		else
			l = 0;
		for ( k = j; j < k + 512 && ( !l || l % 512 ) && j < size; l++, j++ ){
			write_buf[l] = in_buf[j];
		}
		ret = ide_write_sectors( device, 1, i, 0, (unsigned int)write_buf );
	}

	return j;
}

unsigned char ide_read( unsigned char channel, unsigned char reg ){
	unsigned char result;
	if ( reg > 0x07 && reg < 0x0c )
		ide_write( channel, ATA_REG_CONTROL, 0x80 | channels[channel].nien );

	if ( reg < 0x08 )
		result = inb( channels[channel].base + reg - 0x00 );
	else if ( reg < 0x0c )
		result = inb( channels[channel].base + reg - 0x06 );
	else if ( reg < 0x0e )
		result = inb( channels[channel].ctrl + reg - 0x0a );
	else if ( reg < 0x16 )
		result = inb( channels[channel].bmide + reg - 0x0e );

	if ( reg > 0x07 && reg > 0x0c )
		ide_write( channel, ATA_REG_CONTROL, channels[channel].nien );

	return result;
}

void ide_write( unsigned char channel, unsigned char reg, unsigned char data ){
	if ( reg > 0x07 && reg < 0x0c )
		ide_write( channel, ATA_REG_CONTROL, 0x80 | channels[channel].nien );

	if ( reg < 0x08 )
		outb( channels[channel].base + reg - 0x00, data );
	else if ( reg < 0x0c )
		outb( channels[channel].base + reg - 0x06, data );
	else if ( reg < 0x0e )
		outb( channels[channel].ctrl + reg - 0x0a, data );
	else if ( reg < 0x16 )
		outb( channels[channel].bmide + reg - 0x0e, data );

	if ( reg > 0x07 && reg < 0x0c )
		ide_write( channel, ATA_REG_CONTROL, channels[channel].nien );
}

void ide_read_buffer( unsigned char channel, unsigned char reg, unsigned int buffer, unsigned int quads ){
	if ( reg > 0x07 && reg < 0x0c )
		ide_write( channel, ATA_REG_CONTROL, 0x80 | channels[channel].nien );

	asm volatile ( "pushw %es; movw %ds, %ax; movw %ax, %es" );
	if ( reg < 0x08 )
		insl( channels[channel].base + reg - 0x00, buffer, quads );
	else if ( reg < 0x0c )
		insl( channels[channel].base + reg - 0x06, buffer, quads );
	else if ( reg < 0x0e )
		insl( channels[channel].ctrl + reg - 0x0a, buffer, quads );
	else if ( reg < 0x16 )
		insl( channels[channel].bmide + reg - 0x0e, buffer, quads );

	asm volatile ( "popw %es" );

	if ( reg > 0x07 && reg < 0x0c )
		ide_write( channel, ATA_REG_CONTROL, channels[channel].nien );

}

unsigned char ide_polling( unsigned char channel, unsigned int advanced_check ){
	unsigned char state;
	int i = 0;
	for ( i = 0; i < 4; i++ ){
		ide_read( channel, ATA_REG_ALTSTATUS );
	}
	while( ide_read( channel, ATA_REG_STATUS ) & ATA_SR_BSY );
	
	if ( advanced_check ){
		state = ide_read( channel, ATA_REG_STATUS );

		if ( state & ATA_SR_ERR )
			return 2;
		if ( state & ATA_SR_DF  )
			return 1;
		if (( state & ATA_SR_DRQ ) == 0 )
			return 3;
	}
	return 0;
}

unsigned char ide_error( unsigned int drive, unsigned char err ){
	if ( err == 0 )
		return err;
	unsigned char st;

	if ( err == 1 ) printf("[error] IDE device fault\n"); 
	else if ( err == 2 ){
		st = ide_read( ide_devices[drive].channel, ATA_REG_ERROR );

		if ( st & ATA_ER_AMNF ){	printf( "    No address mark found\n" );   err = 7; }
		if ( st & ATA_ER_TK0NF ){	printf( "    No media/media error\n");	   err = 3; }
		if ( st & ATA_ER_ABRT ){	printf( "    Command aborted\n" );	   err = 20;}
		if ( st & ATA_ER_MCR ){		printf( "    No media/media error\n");	   err = 3; }
		if ( st & ATA_ER_IDNF ){	printf( "    ID mark not found\n" );	   err = 21;}
		if ( st & ATA_ER_MC ){		printf( "    No media/media error\n" );	   err = 3; }
		if ( st & ATA_ER_UNC ){		printf( "    Uncorrectable data error\n"); err = 22;}
		if ( st & ATA_ER_BBK ){		printf( "    Bad sectors\n" );		   err = 13;}
	} else if ( err == 3 ){ 		printf( "    Read nothing\n" );		   err = 23;}
	  else if ( err == 4 ){			printf( "    Write protected\n" );	   err = 8; }

	printf( "    %s:%s: %s\n",
		(char *[]){"primary", "secondary"}[ide_devices[drive].channel],
		(char *[]){"master" , "slave"	 }[ide_devices[drive].drive],
		ide_devices[drive].model
	);

	return err;
}

unsigned char ide_ata_access( 	unsigned char direction, unsigned char drive, unsigned int lba,
				unsigned char numsects, unsigned char selector, unsigned int edi ){

	unsigned char lba_mode, dma, cmd;
	unsigned char lba_io[6];
	unsigned int  channel 	= ide_devices[drive].channel;
	unsigned int  slavebit	= ide_devices[drive].drive;
	unsigned int  bus	= channels[channel].base;
	unsigned int  words	= 256;
	unsigned short cyl, i;
	unsigned char head, sect, err;

	ide_write( channel, ATA_REG_CONTROL, channels[channel].nien = (ide_irq_invoked = 0) + 0x2 );

	if ( lba >= 0x10000000 ){
		lba_mode = 2;
		lba_io[0] = (lba & 0x000000ff ) >> 0;
		lba_io[1] = (lba & 0x0000ff00 ) >> 8;
		lba_io[2] = (lba & 0x00ff0000 ) >> 16;
		lba_io[3] = (lba & 0xff000000 ) >> 24;
		lba_io[4] = 0;
		lba_io[5] = 0;
		head	  = 0;
	} else if ( ide_devices[drive].capabilities & 0x200 ){
		lba_mode  = 1;
		lba_io[0] = (lba & 0x0000ff ) >> 0;
		lba_io[1] = (lba & 0x00ff00 ) >> 8;
		lba_io[2] = (lba & 0xff0000 ) >> 16;
		lba_io[3] = 0;
		lba_io[4] = 0;
		lba_io[5] = 0;
		head	  = (lba & 0xf000000) >> 24;
	} else {
		lba_mode  = 0;
		sect	  = (lba % 63) + 1;
		cyl	  = (lba + 1 - sect) / (16 * 63);
		lba_io[0] = sect;
		lba_io[1] = cyl & 0xff;
		lba_io[2] = (cyl >> 8) & 0xff;
		lba_io[3] = 0;
		lba_io[4] = 0;
		lba_io[5] = 0;
		head	  = (lba + 1 - sect) % ( 16 * 63 ) / (63);
	}
	dma = 0;

	while ( ide_read( channel, ATA_REG_STATUS ) & ATA_SR_BSY );

	if ( lba_mode == 0 )
		ide_write( channel, ATA_REG_HDDEVSEL, 0xa0 | (slavebit << 4) | head );
	else
		ide_write( channel, ATA_REG_HDDEVSEL, 0xe0 | (slavebit << 4) | head );


	if ( lba_mode == 2 ){
		ide_write( channel, ATA_REG_SECCOUNT1, 	0);
		ide_write( channel, ATA_REG_LBA3,	lba_io[3]);
		ide_write( channel, ATA_REG_LBA4,	lba_io[4]);
		ide_write( channel, ATA_REG_LBA5,	lba_io[5]);
	}

	ide_write( channel, ATA_REG_SECCOUNT0, 	numsects );
	ide_write( channel, ATA_REG_LBA0,	lba_io[0]);
	ide_write( channel, ATA_REG_LBA1,	lba_io[1]);
	ide_write( channel, ATA_REG_LBA2,	lba_io[2]);

	if ( lba_mode == 0 && dma == 0 && direction == 0 ) cmd = ATA_CMD_READ_PIO;
	if ( lba_mode == 1 && dma == 0 && direction == 0 ) cmd = ATA_CMD_READ_PIO;
	if ( lba_mode == 2 && dma == 0 && direction == 0 ) cmd = ATA_CMD_READ_PIO_EXT;
	if ( lba_mode == 0 && dma == 1 && direction == 0 ) cmd = ATA_CMD_READ_DMA;
	if ( lba_mode == 1 && dma == 1 && direction == 0 ) cmd = ATA_CMD_READ_DMA;
	if ( lba_mode == 2 && dma == 1 && direction == 0 ) cmd = ATA_CMD_READ_DMA_EXT;
	if ( lba_mode == 0 && dma == 0 && direction == 1 ) cmd = ATA_CMD_WRITE_PIO;
	if ( lba_mode == 1 && dma == 0 && direction == 1 ) cmd = ATA_CMD_WRITE_PIO;
	if ( lba_mode == 2 && dma == 0 && direction == 1 ) cmd = ATA_CMD_WRITE_PIO_EXT;
	if ( lba_mode == 0 && dma == 1 && direction == 1 ) cmd = ATA_CMD_WRITE_DMA;
	if ( lba_mode == 1 && dma == 1 && direction == 1 ) cmd = ATA_CMD_WRITE_DMA;
	if ( lba_mode == 2 && dma == 1 && direction == 1 ) cmd = ATA_CMD_WRITE_DMA_EXT;
	ide_write( channel, ATA_REG_COMMAND, cmd );

	if ( dma ){
		if ( direction == 0 );
			//read
		else;
			//write
	} else {
		if ( direction == 0 ){
			for ( i = 0; i < numsects; i++ ){
				if (( err = ide_polling( channel, 1 )))
					return err;

				asm volatile( "pushw %es");
				asm volatile( "mov %%ax, %%es" :: "a"(selector));
				asm volatile( "rep insw":: "c"(words), "d"(bus), "D"(edi));
				asm volatile( "popw %es" );
				edi += ( words * 2 );
			}
		} else {
			//printf( "[6.1] Got here...\n" );
			for ( i = 0; i < numsects; i++ ){
				ide_polling( channel, 0 );
				printf( "" /*More magic*/ );
				asm volatile( "pushw %ds" );
				asm volatile( "mov %%ax, %%ds":: "a"(selector));
				asm volatile( "rep outsw":: "c"(words), "d"(bus), "S"(edi));
				asm volatile( "popw %ds" );
				edi += ( words * 2 );
			}
			ide_write( channel, ATA_REG_COMMAND, (char []){ ATA_CMD_CACHE_FLUSH,
									ATA_CMD_CACHE_FLUSH,
									ATA_CMD_CACHE_FLUSH_EXT}[lba_mode]
			);
			ide_polling( channel, 0 );
		}
	}
	return 0;
}

unsigned char ide_atapi_read( 	unsigned char drive, unsigned int lba, unsigned char numsects, 
				unsigned short selector, unsigned int edi ){
	unsigned int  channel  = ide_devices[drive].channel;
	unsigned int  slavebit = ide_devices[drive].drive;
	unsigned int  bus      = channels[drive].base;
	unsigned int  words    = 1024;
	unsigned int  err;
	int i;

	ide_write( channel, ATA_REG_CONTROL, channels[channel].nien = ide_irq_invoked = 0 );
	atapi_packet[ 0] = ATAPI_CMD_READ;
	atapi_packet[ 1] = 0x0;
	atapi_packet[ 2] = (lba >> 24) & 0xff;
	atapi_packet[ 3] = (lba >> 16) & 0xff;
	atapi_packet[ 4] = (lba >> 8 ) & 0xff;
	atapi_packet[ 5] = (lba >> 0 ) & 0xff;
	atapi_packet[ 6] = 0;
	atapi_packet[ 7] = 0;
	atapi_packet[ 8] = 0;
	atapi_packet[ 9] = numsects;
	atapi_packet[10] = 0;
	atapi_packet[11] = 0;

	ide_write( channel, ATA_REG_HDDEVSEL, slavebit << 4 );

	for ( i = 0; i < 4; i++ )
		ide_read( channel, ATA_REG_ALTSTATUS );

	ide_write( channel, ATA_REG_FEATURES, 0 );
	ide_write( channel, ATA_REG_LBA1, ( words * 2 ) & 0xff );
	ide_write( channel, ATA_REG_LBA2, ( words * 2 ) >> 8 );

	ide_write( channel, ATA_REG_COMMAND, ATA_CMD_PACKET );
	if (( err = ide_polling( channel, 1 ))) return err;

	asm volatile( "rep outsw":: "c"(6), "d"(bus), "S"(atapi_packet));

	for ( i = 0; i < numsects; i++ ){
		ide_wait_irq();
		if (( err = ide_polling( channel, 1 )))
			return err;
		asm volatile( "push %es" );
		asm volatile( "mov %%ax, %%es":: "a"(selector));
		asm volatile( "rep insw":: "c"(words), "d"(bus), "D"(edi));
		asm volatile( "popw %es" );
		edi += ( words * 2 );
	}
	ide_wait_irq();
	while ( ide_read( channel, ATA_REG_STATUS ) & ( ATA_SR_BSY | ATA_SR_DRQ ));

	return 0;
}

void ide_irq_handler( registers_t *regs ){
	ide_irq_invoked = 1;
}

void ide_wait_irq( ){
	while (!ide_irq_invoked);
	ide_irq_invoked = 0;
}

int ide_read_sectors( unsigned char drive, unsigned char numsects, unsigned int lba,
			unsigned short es, unsigned int edi ){

	if ( drive > 3 || ide_devices[drive].reserved == 0 )
		package[0] = 1;

	else if (((lba + numsects) > ide_devices[drive].size ) && ( ide_devices[drive].type == IDE_ATA ))
		package[0] = 2;

	else {
		unsigned char err;
		int i = 0;
		if ( ide_devices[drive].type == IDE_ATA )
			err = ide_ata_access( ATA_READ, drive, lba, numsects, es, edi );
		else if ( ide_devices[drive].type == IDE_ATAPI ){
			for ( i = 0; i < numsects; i++ ){
				err = ide_atapi_read( drive, lba + i, 1, es, edi + (i*2048));
			}
		}
		package[0] = ide_error( drive, err );
	}
	return package[0];
}

int ide_write_sectors( unsigned char drive, unsigned char numsects, unsigned int lba,
			unsigned short es, unsigned int edi ){

	if ( drive > 3 || ide_devices[drive].reserved == 0 )
		package[0] = 1;

	else if (((lba + numsects) > ide_devices[drive].size ) && ( ide_devices[drive].type == IDE_ATA ))
		package[0] = 2;

	else {
		unsigned char err;
		if ( ide_devices[drive].type == IDE_ATA )
			err = ide_ata_access( ATA_WRITE, drive, lba, numsects, es, edi );
		else if ( ide_devices[drive].type == IDE_ATAPI )
			err = 4;
		package[0] = ide_error( drive, err );
	}
	return package[0];
}

#endif
