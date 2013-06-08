#ifndef _kernel_ipc_h
#define _kernel_ipc_h
#include <task.h>
#include <kmacros.h>
enum {
	MSG_NULL,
	MSG_STATUS,
	MSG_ACK,
	MSG_NO_BLOCK,
	MSG_BLOCK,
	MSG_STRING,
	MSG_EXIT,
	MSG_END
};

typedef struct ipc_msg {
	unsigned long msg_type;
	unsigned long sender;
	unsigned long seq;
	struct ipc_msg *next;
	char data[4];
} ipc_msg_t;

void gen_msg( ipc_msg_t *msg, unsigned char type, unsigned long size, void *buf );
int  send_msg( unsigned long pid, ipc_msg_t *msg );
int  get_msg( unsigned long blocking, ipc_msg_t *buf );
char *msg_lookup( unsigned int );

#endif
