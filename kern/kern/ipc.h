#ifndef _kernel_ipc_h
#define _kernel_ipc_h
enum {
	MSG_STATUS,
	MSG_NO_BLOCK,
	MSG_BLOCK,
	MSG_STRING,
	MSG_EXIT,
	MSG_NULL
};

char *msg_lookup( unsigned int );

typedef struct ipc_msg {
	unsigned long msg_type;
	unsigned long sender;
	unsigned long buf_size;
	void *buf;
} ipc_msg_t;

#endif
