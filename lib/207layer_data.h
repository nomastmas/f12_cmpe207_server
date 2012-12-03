#ifndef _207LAYER_H
#define _207LAYER_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <netinet/in.h>

enum FLAGS {CLOSED, LISTEN, SYN_RCVD, SYN_SENT, ESTABLISHED, FIN_WAIT_1, CLOSE_WAIT, FIN_WAIT_2, CLOSING, LAST_ACK, TIME_WAIT};


#define TCP207_SUCCESS 0	/*generic success*/
#define TCP207_ERROR 2	/*generic error*/
#define TCP207_ERROR_NULL_POINTER 3 /*NULL pointer not expected error*/
#define TCP207_CONNECT_WRONG_3WAY_RESPONSE /*Not a valid respnse in 3way handshake*/

#define CONNECT207_SYN 0 	/*SYN state*/
#define CONNECT207_SYN_ACK 1 	/*SYN| ACK state*/
#define CONNECT207_ACK 2 	/*ACK state*/
#define MAX_TCB_SIZE 10

#define SOURCE_PORT 9000

#define MAX_BUF_SIZE 256

struct myTcpBlock
{

	int sock_in_use; 	// socket in use = 1; ready = 0;
	int sockfd_udp; 	//tracks udp sockets
	unsigned short  cmpe207_port;     //htons(3490), 207 port #

	int queue_size;
	
	struct sockaddr_in *pSocket_info;
	struct packet_header *pTcpH;
	struct packet_header *pSentTcpH;
	struct sequence *pSeq;
	int tcp_current_state;
	int tcp_prev_state;
	int window_size; //-->added
	struct sockaddr_in client_address; //-->added

};

struct myTcpBlock gTcp_Block[MAX_TCB_SIZE];

void connect207_print_tcp_header(int tcp_block_index_in);
int tcp_header_extract_from_recv_packet(int tcp_block_index_in, char * pBuffer_in );

#define DEBUG 1

//struct union definition of packet header

// packet header
  struct packet_header {
        unsigned short int source_port;
        unsigned short int dest_port;
        unsigned int       seq_num;
        unsigned int       ack_num;
        unsigned int       data_offset : 5; //SB:Data offset should be 5 as per 207 specification
        unsigned int       reserved    : 2;
        unsigned int       ns_flag     : 1;
        unsigned int       cwr_flag    : 1;
	unsigned int       ece_flag    : 1;
        unsigned int       urg_flag    : 1;
        unsigned int       ack_flag    : 1;
        unsigned int       psh_flag    : 1;
        unsigned int       rst_flag    : 1;
        unsigned int       syn_flag    : 1;
        unsigned int       fin_flag    : 1;
	unsigned short int window_size;
        unsigned short int checksum;
        unsigned short int urg_ptr;
    }__attribute__((packed));

struct sequence
{
	unsigned int send_first_seq_number;/*Seq number of first packet sent by sender that is the process itself*/
	unsigned int send_current_seq_number; /*Seq number of current packet sent by sender that is the process itself*/
	unsigned int send_prev_seq_number;/*Seq number of previous packet sent by sender that is the process itself*/

/*Note that these received sequence numbers will be useful while assigning acknowledgement numbers to the outgoing packets*/
	unsigned int recvd_first_seq_number;/*Seq number of first packet received by sender that is the process itself*/
	unsigned int recvd_current_seq_number;/*Seq number of current packet received by sender that is the process itself*/
	unsigned int recvd_prev_seq_number;/*Seq number of previous packet received by sender that is the process itself*/

};

void die (char *s);
void check_for_error(int ret, char* s);
int get_tcp_state (int tcp_state, struct packet_header recv_header, char* msg);
char* get_state_name (int tcp_state);

#define BACKLOG 10
#define CMPE207_SOC 1
#define CMPE207_PROC 2
#define CMPE207_FAM  3
#define MAX_SOCKET 10
//PORT_UDP = 2100 + group #
#define UDP_PORT 2102
//PORT_207 = 1207 & up
#define CMPE207_PORT_BASE 1207
#define MAX_PORT 5

//tracks 207 ports in use: ready = 0 , in use = 1
extern int cmpe207_port_in_use[MAX_PORT];
/*
typedef struct 
{
	int sock_in_use; 	// socket in use = 1; ready = 0;
	int sockfd_udp; 	//tracks udp sockets
	unsigned short  cmpe207_port;     //htons(3490), 207 port #

	struct sockaddr_in* sock_struct_UDP;

	int tcp_state;
	int queue_size;
	
	struct sequence *pSeq;
	struct packet_header *pTcpH;	
}Control_Block;


extern Control_Block CB[MAX_SOCKET];
*/
// TCP functions

/*
cmpe207_socket
	Function: 	- creates sockfd_207 and sock_fd_UDP
			- sets sock_in_use
	Returns:	- sockfd_207
*/
extern int cmpe207_socket(int ai_family, int ai_socktype, int ai_protocol);

/*
cmpe207_bind
	Function: 	- copies own IP, port, and family to pSocket_info
			- assigns UDP and 207 port #
	Returns:	- UDP bind error code
*/
extern int cmpe207_bind(int sockfd, struct sockaddr_in *addr, socklen_t addrlen);

/*
cmpe207_listen
	Function: 	- queue connections to be made
			- changes socket state from CLOSED to LISTEN
	Returns:	- nothing
*/
extern int cmpe207_listen(int sockfd, int backlog);

/*
cmpe207_accept
	Function: 	- Three-way handshake
			- sockfd is socket created with socket, bind, and listen
	Returns:	- socket
*/
extern int cmpe207_accept(int sockfd, struct sockaddr_in *addr, socklen_t * addrlen);

#endif 
