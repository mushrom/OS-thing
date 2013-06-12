/* Tiny VI, a vi-like editor for my os */
#include <stdio.h>
#include <stdlib.h>
#include <syscall.h>

#define TAB_SIZE 8
#define OVERFLOW_LINES 0
#define MIN( a, b ) ((a<b)?a:b)

typedef enum {
	M_COMMAND = 0,
	M_INSERT = 1,
} emode_t;

typedef struct fbuf {
	int fd;
	int size;
	int lines;
	char *name;
	char *data;
	int *linebuf;
} fbuf_t;

typedef struct screen {
	fbuf_t *buf;
	int fd;
	int in_fd;
	int pos_y;

	int screen_x;
	int screen_y;

	int cur_x;
	int cur_y;
	emode_t mode;
	int running;

	int asdf[8];
} screen_t;

void blit( screen_t *s );
void clear( screen_t *s );
void command_func( screen_t *s, char c );
screen_t *init_screen( int fd, int in );
void insert_func( screen_t *s, char c );
fbuf_t *load_file( char *name );

char *modes[] = { "command", "insert" };
void (*mode_funcs[])( screen_t *s, char c ) = { command_func, insert_func };

void blit( screen_t *s ){
	int i, j, t, k, overflowed;
	char *name = "none";
	char *data = s->buf->data;

	putc( '\n', s->fd );
	/*
	j = 0;
	if ( s->pos_y > 0 ){
	*/
	/*
		for ( i = j = 0; j < s->buf->size; j++ ){
			if ( i == s->pos_y )
				break;
			if ( data[j] == '\n' )
				i++;
		}
	*/
	//}
	//j = t + 1;
	j = s->buf->linebuf[ MIN( s->pos_y, s->buf->lines )];

	for ( i = 0; i < s->screen_y - 1; i++ ){
		overflowed = 0;
		for ( t = j + s->screen_x; j < t && j < s->buf->size; j++ ){
			if ( data[j] == '\n' ){
				j++;
				break;
			} else if ( data[j] == '\t' ){
				k = ( s->screen_x - (t - j ) + 1) % TAB_SIZE;
				for ( ; k <= TAB_SIZE && j <= t; t--, k++ ){
					putc( ' ', s->fd );
					//t -= 1;
				}
			} else {
				putc( data[j], s->fd );
			}

			if ( j == t - 1 )
				overflowed  = 1;

		}

		if ( !OVERFLOW_LINES && overflowed ){
			while ( data[j] != '\n' && j < s->buf->size )
				j++;

			j++;
			//if ( data[j] == '\n' )j++;
		}

		putc( '\n', s->fd );

		if ( j == s->buf->size )
			putc( '~', s->fd );

	}

	if ( s->buf )
		name = s->buf->name;

	printf( "[%s:%d] (%s): %d, %d, %d", name, s->buf->lines, modes[s->mode], s->cur_x, s->cur_y, s->pos_y );
}

void clear( screen_t *s ){
	int i;
	for ( i = s->pos_y ; i < s->screen_y - 1; i++ )
		putc( '\n', s->fd );
}

void command_func( screen_t *s, char c ){
	switch ( c ){
		case 'j':
			s->pos_y++;
			break;
		case 'k':
			if ( s->pos_y )
				s->pos_y--;
			break;
		case 'g':
			s->pos_y = 0;
			break;
		case 'G':
			s->pos_y = s->buf->lines - s->screen_y;
			if ( s->pos_y < 0 )
				s->pos_y = 0;
			break;
		case 'i':
			s->mode = M_INSERT;
			break;
		case 'q':
			s->running = 0;
			break;
	}
	return;
}

screen_t *init_screen( int fd, int in ){
	screen_t *ret = malloc( sizeof( screen_t ));
	memset( ret, 0, sizeof( screen_t ));

	ret->fd = fd;
	ret->in_fd = in;
	//ret->mode = M_COMMAND;
	ret->running = 1;

	ret->screen_x = 80;
	ret->screen_y = 25;

	ret->pos_y = 0;

	return ret;
}

void insert_func( screen_t *s, char c ){
	switch( c ){
		case 033:
			s->mode = M_COMMAND;
			break;
	}
	return;
}

fbuf_t *load_file( char *name ){
	fbuf_t *ret = malloc( sizeof( fbuf_t ));
	struct vfs_stat st;
	int j, i, lines;

	memset( ret, 0, sizeof( fbuf_t ));
	memset( &st, 0, sizeof( struct vfs_stat ));

	int fd = open( name, O_CREAT | O_RDONLY );
	syscall_lstat( name, &st );
	
	ret->fd = fd;
	ret->name = name;
	ret->data = malloc( st.size + 1 );
	ret->size = st.size;
	read( fd, ret->data, st.size );

	for ( lines = i = 0; i < st.size; i++ ){
		if ( ret->data[i] == '\n' )
			lines++;
	}
	ret->lines = lines;

	ret->linebuf = malloc( sizeof( int ) * ( lines + 16 ));
	for ( j = i = 0; i < st.size && j < lines; i++ ){
		if ( ret->data[i] == '\n' )
			ret->linebuf[j++] = i + 1; //+ 1;
	}
	
	//write( stdout, ret->data, st.size );
	//printf( "%d\n", st.size );

	return ret;
}

int main( int argc, char *argv[], char *envp[] ){
	if ( argc < 2 ){
		printf( "usage: %s [file]\n", argv[0] );
		return 0;
	}

	//int serial = open( "/dev/ser0", O_WRONLY | O_RDONLY );

	screen_t *s = init_screen( stdout, stdin );
	fbuf_t *meh = load_file( argv[1] );
	s->buf = meh;
	//meh->name = "asdf";
	//s->buf = 0;

	//clear( s );
	char c = 0;

	//return 0;
	blit( s );
	while( s->running ){
		c = getc( s->in_fd );
		mode_funcs[ s->mode ]( s, c );
		blit( s );
	}

	close( s->buf->fd );
	free( s->buf->data );
	free( s->buf );
	free( s );
	
	return 0;
}
