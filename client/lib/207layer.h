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
//struct union definition of packet header


// packet header
// may not need a union
typedef union {
    char full_20_byte[20];
    // assuming little endian other byte order is swapped
    struct {
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
} packet_header;

void die (char *s);
void check_for_error(int ret, char* s);
// msg is for any additional parameter that won't fit in packet_header, such as close
int get_tcp_state (int tcp_state, packet_header recv_header, char* msg);
char* get_state_name (int tcp_state);

#if 0
#define __BYTE_ORDER __LITTLE_ENDIAN

/* 207 TCP Header: 20 bytes  */
struct 207TcpHdr {
        unsigned short srcPort;     	/* Source port number: 16 bit */
        unsigned short destPort;	/* Destination port number : 16 bit */
        unsigned long seq;		/* Sequence Number: 32 bits */
        unsigned long ackSeq;  		/* Acknowledgement number: 32 bits */
        
	#  if __BYTE_ORDER == __LITTLE_ENDIAN
        unsigned short res1:3;		/* Reserved: 3 upper bits */	
        unsigned short dataOff:5;	/* Data Offset 207 specific: 5 bits */
        unsigned short fin:1;		/* Finish bit: 1 bit */
        unsigned short syn:1;		/* Synchronize bit: 1 bit */
        unsigned short rst:1;		/* Reset connection bit: 1 bit */
        unsigned short psh:1;		/* Push bit: 1 bit */
        unsigned short ack:1;		/* Acknowledgement bit: 1 bit */
        unsigned short urg:1;		/* Urgent bit: 1 bit */
        unsigned short res2:2;		/* Reserved bits: 2 lower bits */
        
	#  elif __BYTE_ORDER == __BIG_ENDIAN
        unsigned short doff:5;		/* Data Offset 207 specific: 5 bits */
        unsigned short res1:3;		/* Reserved: 3 upper bits */
        unsigned short res2:2;		/* Reserved bits: 2 lower bits */
        unsigned short urg:1;		/* Urgent bit: 1 bit */
        unsigned short ack:1;		/* Acknowledgement bit: 1 bit */
        unsigned short psh:1;		/* Push bit: 1 bit */
        unsigned short rst:1;		/* Reset connection bit: 1 bit */
        unsigned short syn:1;		/* Finish bit: 1 bit */
        unsigned short fin:1;		/* Finish bit: 1 bit */
        
	#  endif
        unsigned short window;  	/* Window: 16 bit */
        unsigned short check;		/* Checksum: 16 bit */
        unsigned short urgPtr;		/* Urgent Pointer: 16 bit */
};

/*207 TCP 40 bytes header*/
union 207TcpHdr40 
{
	struct 207TcpHdr 207Hdr;
	unsigned long options[5];
};


#endif

// client specific functions
// int connect(int sockfd, struct sockaddr *serv_addr, int addrlen); 

// server specific functions
// int bind(int sockfd, const struct sockaddr *addr, socklen_t addrlen);
// int listen(int sockfd, int backlog);
// int accept(int sockfd, struct sockaddr *addr, socklen_t *addrlen);

// common functions
// ssize_t recv(int sockfd, void *buf, size_t len, int flags);
// ssize_t send(int sockfd, const void *buf, size_t len, int flags);
// int close(int fd);

void die (char *s);
void check_for_error(int ret, char* s);

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

typedef struct 
{
	int sock_in_use; 	// socket in use = 1; ready = 0;
	int sockfd_udp; 	//tracks udp sockets
	unsigned short  cmpe207_port;     //htons(3490), 207 port #

	struct sockaddr_in sock_struct_UDP;
}Control_Block;

extern Control_Block CB[MAX_SOCKET];

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
	Function: 	- copies own IP, port, and family to sock_struct_UDP
			- assigns UDP and 207 port #
	Returns:	- UDP bind error code
*/
extern int cmpe207_bind(int sockfd, struct sockaddr_in *addr, socklen_t addrlen);


#endif 
