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

  struct packet_header{
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
#endif
