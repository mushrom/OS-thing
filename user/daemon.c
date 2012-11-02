#include "syscall.h"
#include "ipc.h"

void listen( void ){
	ipc_msg_t msg;
	int fd, ret = 0, i;
	char *buf;
	while ( 1 ){
		ret = syscall_get_msg( MSG_BLOCK, &msg );
		if ( !ret ){
			if ( msg.msg_type == MSG_EXIT ){
				syscall_kputs( "Mmk, I'm out\n" );
				syscall_exit( 0 );
			}
			if ( msg.msg_type == MSG_STATUS ){
				syscall_kputs( "Herro, I'm a userland program. :3\n" );
			}
			ret = 0;
			buf = (char *)&msg;
			for ( i = 0; i < sizeof( ipc_msg_t ); i++ )
				buf[i] = 0;
		}
	}
}
		

int main( void ){
	syscall_kputs( "Starting listening daemon...\n" );
	syscall_thread( &listen );
	syscall_kputs( "Okay, done, I'm out\n" );
	return 0;
}
