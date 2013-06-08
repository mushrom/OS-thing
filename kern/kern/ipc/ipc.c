#ifndef _kernel_ipc_c
#define _kernel_ipc_c
#include <ipc.h>

char *msg_lookup_table[] = {
	"null"
	"status",
	"acknowledge",
	"no block",
	"block",
	"string",
	"exit",
	"end"
};

extern task_t *current_task;

/** \brief Recieve a message from another process
 * @param buf Buffer to read message into
 * @param blocking Whether to block the thread or not
 * @return 0 if a message was recieved, or 1 if there were no messages.
 */
int get_msg( unsigned long blocking, ipc_msg_t *buf ){
	asm volatile( "cli" );
	task_t *temp = (task_t *)current_task;
	int ret = 1;
	asm volatile( "sti" );

	ipc_msg_t *msg = current_task->msg_buf;

	do {
		asm volatile( "cli" );
		if ( temp->msg_count ){
			if ( !msg ){
				DEBUG_HERE
				PANIC( "Caught null pointer in get_msg()" );
			}

			memcpy( buf, msg, sizeof( ipc_msg_t ));

			temp->msg_count--;
			msg = msg->next;
			kfree( temp->msg_buf );
			temp->msg_buf = msg;

			ret = 0;
			break;
		} else {
			ret = 1;
			temp->status = S_LISTENING;
			switch_task();
		}
		asm volatile( "sti" );
	} while ( blocking && !ret );

	asm volatile( "sti" );
	return ret;
}

/** \brief Send message to another process 
 * @param pid The pid to send to
 * @param msg Pointer to message buffer to send
 * @return 0 if sent, or 1 if the pid couldn't be found
 */
int send_msg( unsigned long pid, ipc_msg_t *buf ){
	asm volatile( "cli" );
	task_t *temp = get_pid_task( pid );

	if ( !temp ){
		asm volatile( "sti" );
		return 1;
	}

	ipc_msg_t *msg = (void *)kmalloc( sizeof( ipc_msg_t ), 0, 0 );
	ipc_msg_t *ptr = temp->msg_buf;

	memcpy( msg, buf, sizeof( ipc_msg_t ));

	if ( !ptr ){
		temp->msg_buf = msg;
	} else {
		while ( ptr->next )
			ptr = ptr->next;

		ptr->next = msg;
	}

	temp->msg_count++;

	asm volatile( "sti" );
	return 0;
}


char *msg_lookup( unsigned int index ){
	if ( index < MSG_END )
		return msg_lookup_table[ index ];
	else
		return "unknown";
}

#endif
