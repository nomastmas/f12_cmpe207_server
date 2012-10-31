#ifndef _207LAYER_H
#define _207LAYER_H

//struct union definition of packet header


enum FLAGS {CLOSED, LISTEN, SYN_RCVD, SYN_SENT, ESTABLISHED, FIN_WAIT_1, CLOSE_WAIT, FIN_WAIT_2, CLOSING, LAST_ACK, TIME_WAIT};


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
int get_tcp_state (int tcp_state, packet_header recv_packet);
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

#endif 
