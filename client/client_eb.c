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

#define MAX 256

void die (char *s){
	perror(s);
	exit(1);
}

void check_for_error(int ret, char* s){
	if (ret < 0){
		die (s);
	}
}

typedef union {
    char full_20_byte[20]; 
    // assuming little endian other byte order is swapped
    struct {
        unsigned short int source_port;
        unsigned short int dest_port;
        unsigned int seq_num;
        unsigned int ack_num;
        unsigned int data_offset : 4; //SB:Data offset should be 5 as per 207 specification
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
} packet_header;


int main (int argc, char *argv[]){

	if ( argc != 3 ){
		die("usage: client hostname");
	}

	int sockfd, ret, slen;
	char buf[MAX];
	struct addrinfo hints, *servinfo, *p;
	struct sockaddr_in s_server;

	const char *port = argv[2];

	slen = sizeof(s_server);

	bzero (&s_server,sizeof(s_server));
	s_server.sin_family = AF_INET;
	s_server.sin_addr.s_addr = inet_addr (argv[1]);	//inet_addr not recommended
	s_server.sin_port	= htons (atoi(argv[2]));

	printf ("...booting up client...\n");

	sockfd = socket (AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	if (sockfd == -1) {
		die("socket()");
	}

	printf ("send message to UDP echo server:\n");
//	fgets (buf, MAX, stdin); //---eb removed for test

// ---this section was added to send a test header---

	packet_header header;

	header.source_port 	= 2000;
	header.dest_port 		= 2100;
	header.seq_num 		= 1;
	header.ack_num 		= 0;
	header.data_offset 	= 5; 
	header.reserved 		= 1;
	header.ns_flag 		= 0;
	header.cwr_flag 		= 0;
	header.ece_flag 		= 0;
	header.urg_flag 		= 0;
	header.ack_flag 		= 0;
	header.psh_flag 		= 0;
	header.rst_flag 		= 0;
	header.syn_flag 		= 1;
	header.fin_flag 		= 0;
	header.window_size 	= 128;
	header.checksum 		= 0;
	header.urg_ptr 		= 0;

	printf("Source Port\t\t%d\n", 	header.source_port);
	printf("Dest Port\t\t%d\n", 		header.dest_port);
	printf("Seq Num\t\t\t%d\n", 		header.seq_num);
	printf("Ack Num\t\t\t%d\n", 		header.ack_num);
	printf("Data Offset\t\t%d\n", 	header.data_offset);
	printf("Reserved\t\t%d\n", 		header.reserved);
	printf("NS Flag\t\t\t%d\n", 		header.ns_flag);
	printf("CWR Flag\t\t%d\n", 		header.cwr_flag);
	printf("ECE Flag\t\t%d\n", 		header.ece_flag);
	printf("URG Flag\t\t%d\n", 		header.urg_flag);
	printf("ACK Flag\t\t%d\n", 		header.ack_flag);
	printf("PSH Flag\t\t%d\n", 		header.psh_flag);
	printf("RST Flag\t\t%d\n", 		header.rst_flag);
	printf("SYN Flag\t\t%d\n", 		header.syn_flag);
	printf("FIN Flag\t\t%d\n", 		header.fin_flag);
	printf("Window Size\t\t%d\n", 	header.window_size);
	printf("Checksum\t\t%d\n", 		header.checksum);
	printf("Urgent Ptr\t\t%d\n", 		header.urg_ptr);

	ret = sendto (sockfd, &header, MAX, 0, (struct sockaddr*)&s_server, slen);

//	ret = sendto (sockfd, buf, MAX, 0, (struct sockaddr*)&s_server, slen); //---eb removed for test
	check_for_error (ret, "sendto()");

//	ret = recvfrom (sockfd, buf, MAX, 0, (struct sockaddr*)&s_server, &slen);
//	check_for_error (ret, "recvfrom()");

//	printf ("==response==\n%s\n", buf);
	printf ("goodbye.\n");

	close (sockfd);
	return 0;
}
