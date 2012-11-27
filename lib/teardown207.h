#ifndef TEARDOWN207_H
#define TEARDOWN207_H


#define TCP207_SUCCESS 0	/*generic success*/
#define TCP207_ERROR 2	/*generic error*/
#define TCP207_ERROR_NULL_POINTER 3 /*NULL pointer not expected error*/
#define TCP207_CONNECT_ERROR_WRONG_3WAY_RESPONSE 4 /*Not a valid respnse in 3way handshake*/
#define TCP207_ERROR_TCPSTATE_MISBEHAVE 5 /*TCP state machine misbehave*/

#define ACK207 10
#define FIN207 11


int teardown207(int tcp_block_index_in, int clientOrServer_in  );

#endif
