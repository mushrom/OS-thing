#include <syscall.h>
#include <fs.h>

int main( int argc, char *argv[] ){
	int fp, items = 0;
	char *listing = ".";

	if ( argc > 1 )
		listing = argv[1];

	fp = open( listing, O_RDONLY );
	struct dirp d, *dir;
	dir = fdopendir_c( fp, &d );
	struct dirent entry;
	struct vfs_stat st;

	char *types[] = {
		"file",
		"dir",
		"char",
		"block",
		"pipe",
		"sym",
		"mount"
	};

	if ( dir ){
		printf( "name		type	size	uid	gid	mask\n" );
		while (( readdir_c( fp, dir, &entry ))){
			int tsize = strlen( listing ) + strlen( entry.name ) + 3;
			char *buf = malloc( tsize );
			memset( buf, 0, tsize );

			strcpy( buf, listing );
			buf[ strlen( listing ) ] = '/';
			strcpy( buf + strlen( listing ) + 1, entry.name );
			//printf( "[debug] getting stats for \"%s\".\n", buf );
			syscall_lstat( buf, &st );
			free( buf );

			int color = 0x11 + st.type;

			//printf( "%c%s%c\n", color, entry.name, 0x11 );
			printf( "%c%s%c\t\t%s\t%d\t%d\t%d\t%d\n", color, entry.name, 0x17, types[ st.type ], 
				st.size, st.uid, st.gid, st.mask );
			/*
			if ( ++items % 8 == 0 )
				printf( "\n" );
			*/
		}
		//printf( "\n" );
		close( fp );
	} else {
		printf( "Could not open directory.\n" );
	}

	printf( "%c", 0x17 );
	return 0;
}
