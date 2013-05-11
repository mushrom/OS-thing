#ifndef _kernel_symbols_h
#define _kernel_symbols_h
#include <stdio.h>
#include <alloc.h>

typedef struct symbol {
	unsigned long hash;
	unsigned long address;
	struct symbol *next;
} ksymbol_t;

typedef struct symbol_bucket {
	unsigned long entries;
	struct symbol *root;
} ksymbol_bucket_t;

typedef struct symbol_bin {
	unsigned long nbuckets;
	unsigned long used;
	struct symbol_bucket *buckets;
} ksymbol_bin_t;

ksymbol_bin_t *init_symbol_bin( unsigned long size );
unsigned long hash_symbol( char *string );

int export_symbol( ksymbol_bin_t *bin, char *string, unsigned long address );
unsigned long get_symbol( ksymbol_bin_t *bin, char *string );

int kexport_symbol( char *string, unsigned long address );
unsigned long kget_symbol( char *string );

#endif
