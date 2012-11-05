#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <netinet/in.h>

#include <pthread.h>
#include <unistd.h>

#include <time.h>
#include <sys/time.h>


//typedef union {
//    char full_20_byte[20]; 
    // assuming little endian other byte order is swapped
    struct tcp_header {
        unsigned short int source_port;
        unsigned short int dest_port;
        unsigned int seq_num;
        unsigned int ack_num;
        unsigned int data_offset : 4;
        unsigned int reserved : 3;
        unsigned int ns_flag :  1;
        unsigned int cwr_flag : 1;
        unsigned int ece_flag : 1;
        unsigned int urg_flag : 1;
        unsigned int ack_flag : 1;
        unsigned int psh_flag : 1;
        unsigned int rst_flag : 1;
        unsigned int syn_flag : 1;
        unsigned int fin_flag : 1;
        unsigned short int window_size;
        unsigned short int checksum;
        unsigned short int urg_ptr;
    }__attribute__((packed));
//} packet_header;

char segment[576];
struct tcp_header *header = (struct tcp_header *) segment;

//----------------------------------------------------------
//                TCP Control Block
//----------------------------------------------------------

struct tcp_control_block {
	int sd;
	struct sockaddr_in dest_addr;
};

struct tcp_control_block server_tcb[10] = {0};

//----------------------------------------------------------
//                  Send Buffer
//----------------------------------------------------------

const unsigned char *send_buffer = "This is a test message..."; //not sure where this belongs

//----------------------------------------------------------
//              Fuctions - back into header file
//----------------------------------------------------------


void die (char *s){
	perror(s);
	exit(1);
}

void check_for_error(int ret, char* s){
	if (ret < 0){
		die (s);
	}
}

void print_header (struct tcp_header *header);
int send_207(int sockfd, const unsigned char *buffer, uint32_t buffer_size, int flags);

//----------------------------------------------------------
//                  Program Main
//----------------------------------------------------------

int main (int argc, char *argv[]){

//----------------------------------------------------------
//              Initialize the Header
//----------------------------------------------------------
	header->source_port 	= 2000;
	header->dest_port 	= 2100;
	header->seq_num 		= 1;
	header->ack_num 		= 0;
	header->data_offset 	= 5; 
	header->reserved 		= 1;
	header->ns_flag 		= 0;
	header->cwr_flag 		= 0;
	header->ece_flag 		= 0;
	header->urg_flag 		= 0;
	header->ack_flag 		= 0;
	header->psh_flag 		= 0;
	header->rst_flag 		= 0;
	header->syn_flag 		= 1;
	header->fin_flag 		= 0;
	header->window_size 	= 128;
	header->checksum 		= 0;
	header->urg_ptr 		= 0;

//print_header (header);

	if ( argc != 3 ){
		die("usage: client hostname");
	}

	int sockfd;
	struct sockaddr_in server_address;

	bzero (&server_address,sizeof(server_address));
	server_address.sin_family = AF_INET;
	server_address.sin_addr.s_addr = inet_addr (argv[1]);	//inet_addr not recommended
	server_address.sin_port	= htons (atoi(argv[2]));

	sockfd = socket (AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	if (sockfd == -1) {
		die("socket()");
	}

//this gets filled in earlier....
	server_tcb[0].sd = sockfd;
	server_tcb[0].dest_addr = server_address;

// '3' is hard-coded; not sure how to input the socket descriptor for the correct underlying
// upd socket; need error checking, probably in the send_207 function
	int return_value = send_207(3, send_buffer, sizeof(send_buffer), 0); 			  
	printf("sent %d bytes...\n", return_value);

	close (sockfd);
	return 0;
}

//----------------------------------------------------------
//               Function Definitions
//----------------------------------------------------------

void print_header (struct tcp_header *header)
{
	printf("Source Port\t\t%d\n", 	header->source_port);
	printf("Dest Port\t\t%d\n", 		header->dest_port);
	printf("Seq Num\t\t\t%d\n", 		header->seq_num);
	printf("Ack Num\t\t\t%d\n", 		header->ack_num);
	printf("Data Offset\t\t%d\n", 	header->data_offset);
	printf("Reserved\t\t%d\n", 		header->reserved);
	printf("NS Flag\t\t\t%d\n", 		header->ns_flag);
	printf("CWR Flag\t\t%d\n", 		header->cwr_flag);
	printf("ECE Flag\t\t%d\n", 		header->ece_flag);
	printf("URG Flag\t\t%d\n", 		header->urg_flag);
	printf("ACK Flag\t\t%d\n", 		header->ack_flag);
	printf("PSH Flag\t\t%d\n", 		header->psh_flag);
	printf("RST Flag\t\t%d\n", 		header->rst_flag);
	printf("SYN Flag\t\t%d\n", 		header->syn_flag);
	printf("FIN Flag\t\t%d\n", 		header->fin_flag);
	printf("Window Size\t\t%d\n", 	header->window_size);
	printf("Checksum\t\t%d\n", 		header->checksum);
	printf("Urgent Ptr\t\t%d\n", 		header->urg_ptr);
}

int send_207(int sockfd, const unsigned char *buffer, uint32_t buffer_size, int flags)
{	
	int i;
	int index;
	for (i = 0; i < 10; i++){
		if (server_tcb[i].sd == sockfd)
			index = i;
	}

	int socket = server_tcb[index].sd;
	char *payload;
	payload = &segment[sizeof (struct tcp_header)];
	strcpy(payload, buffer);

	struct sockaddr_in dest = server_tcb[index].dest_addr;
	int rv = sendto(sockfd, segment, sizeof(segment), flags, (struct sockaddr *) &dest, sizeof(dest));
	return rv;
}


