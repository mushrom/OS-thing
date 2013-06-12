#ifndef _kernel_initrd_h
#define _kernel_initrd_h
#define INITRD_MAX_INODES 	64
#define INITRD_MAGIC		0xbaabaaba

#include <fs.h>
#include <stdio.h>
#include <stdint.h>

typedef struct initrd_header {
	uint32_t nfiles;
	uint32_t magic;
} initrd_header_t;

typedef struct initrd_file_header {
	char name[ MAX_NAME_LEN ];
	uint32_t offset;
	uint32_t length;
	uint32_t magic;
	uint8_t  type;
} initrd_file_header_t;

file_node_t *initrd_create( initrd_header_t * );

#endif
