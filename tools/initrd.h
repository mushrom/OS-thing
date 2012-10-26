#ifndef _user_initrd_h
#define _user_initrd_h
#define INITRD_MAGIC		0xbaabaaba
#define MAX_NAME_LEN		256	

typedef struct initrd_header {
	uint32_t nfiles;
	uint32_t magic;
} initrd_header_t;

typedef struct initrd_file_header {
	char name[ MAX_NAME_LEN ];
	uint32_t offset;
	uint32_t length;
	uint32_t magic;
} initrd_file_header_t;

#endif
