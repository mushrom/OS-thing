#ifndef _kernel_ipc_c
#define _kernel_ipc_c
#include <kern/ipc.h>

char *msg_lookup_table[] = {
	"status",
	"no block",
	"block",
	"string",
	"exit",
	"null"
};

char *msg_lookup( unsigned int index ){
	if ( index < MSG_NULL )
		return msg_lookup_table[ index ];
	else
		return "unknown";
}

#endif
