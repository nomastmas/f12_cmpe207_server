#ifndef CONNECT207_H
#define CONNECT207_H


#define TCP207_SUCCESS 0	/*generic success*/
#define TCP207_ERROR 2	/*generic error*/
#define TCP207_ERROR_NULL_POINTER 3 /*NULL pointer not expected error*/
#define TCP207_CONNECT_WRONG_3WAY_RESPONSE /*Not a valid respnse in 3way handshake*/

#define CONNECT207_SYN 0 	/*SYN state*/
#define CONNECT207_SYN_ACK 1 	/*SYN| ACK state*/
#define CONNECT207_ACK 2 	/*ACK state*/
#define MAX_TCB_SIZE 10



struct myTcpBlock
{
	int sockfd;
	struct sockaddr_in *pSocket_info;
	struct packet_header *pTcpH;
	struct packet_header *pSentTcpH;
	struct sequence *pSeq;
	int tcp_current_state;
	int tcp_prev_state;

};

struct myTcpBlock gTcp_Block[MAX_TCB_SIZE];

void connect207_print_tcp_header(int tcp_block_index_in);
int tcp_header_extract_from_recv_packet(int tcp_block_index_in, char * pBuffer_in );
#endif
