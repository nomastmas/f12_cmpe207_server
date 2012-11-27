#ifndef CONNECT207_H
#define CONNECT207_H


#define TCP207_SUCCESS 0	/*generic success*/
#define TCP207_ERROR 2	/*generic error*/
#define TCP207_ERROR_NULL_POINTER 3 /*NULL pointer not expected error*/
#define TCP207_CONNECT_ERROR_WRONG_3WAY_RESPONSE 4 /*Not a valid respnse in 3way handshake*/
#define TCP207_ERROR_TCPSTATE_MISBEHAVE 5 /*TCP state machine misbehave*/

#define CONNECT207_SYN 0 	/*SYN state*/
#define CONNECT207_SYN_ACK 1 	/*SYN| ACK state*/
#define CONNECT207_ACK 2 	/*ACK state*/
#define MAX_TCB_SIZE 10



void connect207_print_tcp_header(int tcp_block_index_in);

int connect207(int tcp_block_index_in, int clientOrServer_in  );
#endif
