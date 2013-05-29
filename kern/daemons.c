#ifndef _kernel_daemons_c
#define _kernel_daemons_c
#include <daemons.h>

/** A test daemon that listens for a message, and executes commands
 * based on the message
 */
void misc_daemon_1( ){
	ipc_msg_t msg;
	int ret, meh;
	while ( 1 ){
		ret = get_msg( MSG_BLOCK, &msg );
		if ( ret == 0 ){
			meh = 0xfffff;
			switch ( msg.msg_type ){
				case MSG_EXIT:
					reboot();
					break;
				case MSG_STATUS:
					printf( "Main daemon, how can I help you?\n" );
					while( meh-- );
					break;
			}
			printf( "meh\n" );
		}
	}
}

/** A process to test the ipc's messaging */
void misc_daemon_2( ){
	ipc_msg_t msg, reply;
	int ret;
	while ( 1 ){
		ret = get_msg( MSG_NO_BLOCK, &msg );
		if ( ret == 0 ){
			printf( "pid %d: got message 0x%x (%s) from pid %d\n",
				 getpid(), msg.msg_type, msg_lookup( msg.msg_type ), msg.sender );

			if ( msg.msg_type == MSG_STATUS ){
				reply.msg_type 	= MSG_ACK;
				reply.sender	= getpid();
				send_msg( msg.sender, &reply );
			}

			if ( msg.msg_type == 123 )
				exit_thread();

			if ( msg.msg_type == 234 ){
				int fd = open( "/init/blarg", 0 );
				if ( fd > -1 ){
					printf( "Testing...(%d)\n", fd );
					fexecve( fd, 0, 0 );
				}
				close( fd );
				printf( "Complete\n" );
			}
		} else {
			//sleep_thread( 1 );
		}
	}
	exit_thread();
}

#endif
