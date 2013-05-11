#ifndef _kernel_ipc_c
#define _kernel_ipc_c
#include <symbols.h>

#define new( N ) (memset((void *)kmalloc( sizeof( N ), 0, 0 ), 0, sizeof( N )))

ksymbol_bin_t *ksymbol_table;

unsigned long hash_symbol( char *string ){
	unsigned long ret, i;

	for ( ret = i = 0; string[i]; i++ ){
		ret ^= string[i] * (i + 1);
		ret *= string[i];
	}

	return ret;
}

ksymbol_t *make_symbol( char *string, unsigned long address ){
	ksymbol_t *ret = new( ksymbol_t );

	ret->hash = hash_symbol( string );
	ret->address = address;

	return ret;
}

unsigned long get_symbol( ksymbol_bin_t *bin, char *string ){
	unsigned long hash = hash_symbol( string );

	ksymbol_bucket_t *b = &bin->buckets[hash % bin->nbuckets];
	ksymbol_t *ptr = b->root;

	printf( "[sym] %d\n", hash % bin->nbuckets );

	if ( b->entries == 0 ){
		return 0;
	} else {
		for ( ; ptr; ptr = ptr->next ){
			if ( ptr->hash == hash )
				return ptr->address;
		}
	}

	return 0;
}

int export_symbol( ksymbol_bin_t *bin, char *string, unsigned long address ){
	unsigned long hash = hash_symbol( string );
	int found;

	ksymbol_bucket_t *b = &bin->buckets[hash % bin->nbuckets];
	ksymbol_t *ptr, *new_node;

	ptr = b->root;

	if ( b->entries == 0 ){
		bin->used++;
		b->entries++;
		b->root = make_symbol( string, address );
		printf( "[sym] allocated new root %d\n", hash % bin->nbuckets );
	} else {
		for ( found = 0; ptr && !found; ptr = ptr->next ){
			new_node = ptr;
			if ( hash == ptr->hash ){
				ptr->address = address;
				printf( "[sym] exported over old symbol\n" );
				found = 1;
				break;
			}
		}
		if ( !found ){
			printf( "[sym] allocating new node\n" );
			new_node->next = make_symbol( string, address );

			b->entries++;
		}
	}
	
	return 0;
}

int kexport_symbol( char *string, unsigned long address ){
	return export_symbol( ksymbol_table, string, address );
}

unsigned long kget_symbol( char *string ){
	return get_symbol( ksymbol_table, string );
}

ksymbol_bin_t *init_symbol_bin( unsigned long size ){
	ksymbol_bin_t *ret = new( ksymbol_bin_t );

	ret->nbuckets = size;
	ret->buckets = new( ksymbol_bucket_t[size] );

	return ret;
}

#endif
