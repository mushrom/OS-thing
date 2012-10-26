#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include "initrd.h"

int main( int argc, char *argv[] ){
	FILE *in_fp, *out_fp;
	struct stat sb;
	initrd_header_t header;
	initrd_file_header_t file_buf;
	int i = 0, j = 0, offset = 0;
	char buf = 0;

	memset( &header,   0, sizeof( header   ));
	memset( &file_buf, 0, sizeof( file_buf ));

	if ( argc < 3 )	{
		printf( "Usage: %s [output] [files]\n", argv[0] );
		return 0;
	}

	out_fp = fopen( argv[1], "w" );

	header.nfiles = argc - 2;
	header.magic  = INITRD_MAGIC;

	fwrite( &header, sizeof( initrd_header_t ), 1, out_fp );

	for ( i = 2; i < argc; i++ ){
		j = stat( argv[i], &sb );
		if ( !j && S_ISREG( sb.st_mode )){
			file_buf.length = sb.st_size;
			file_buf.offset = offset;
			strncpy( file_buf.name, argv[i], MAX_NAME_LEN );
			file_buf.magic = INITRD_MAGIC;
			offset += sb.st_size;
		
			fwrite( &file_buf, sizeof( initrd_file_header_t ), 1, out_fp );
		}
	}
	for ( i = 2; i < argc; i++ ){
		if (( in_fp = fopen( argv[i], "r" )) == NULL ){
			printf( "Could not generate image, \"%s\" does not exist\n", argv[i] );
			exit( 1 );
		}
		while (( buf = fgetc( in_fp )) != EOF && !feof( in_fp ))
			fputc( buf, out_fp );
	}

	return 0;
}
