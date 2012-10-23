#ifndef _kernel_ipc_h
#define _kernel_ipc_h
enum {
	MSG_DUMMY,
	MSG_STRING,
	MSG_MEH
};

typedef struct ipc_msg {
	unsigned long msg_type;
	unsigned long sender;
	unsigned long buf_size;
	void *buf;
} ipc_msg_t;

#endif
