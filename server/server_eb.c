#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <pthread.h>

#include <sys/types.h>
#include <ifaddrs.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define MAX 512
#define PORT 9999

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
        unsigned int data_offset : 4; //eb: bits is 4; value is 5
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


struct t_data{
	int fd;
	char* buffer;
	struct sockaddr_in* client;
	int slen;
	int ret;
};

void get_self_ip (char* addressBuffer);
void* rw (void * data);


int main (void){

	printf ("...booting up...\n");

	struct sockaddr_in s_server, s_client;
	int sockfd, ret, slen, t_good;
	char buf[MAX];
	struct t_data rw_data;
	pthread_t t_id;
	char self_addr[INET_ADDRSTRLEN];

	slen = sizeof(s_client);

	sockfd = socket (AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	if (sockfd < 0) {
		die ("socket()");
	}

	memset((char *) &s_server, 0, sizeof(s_server));
	s_server.sin_family = AF_INET;
  	s_server.sin_port = htons(PORT);
  	s_server.sin_addr.s_addr = htonl(INADDR_ANY);	
  	if (bind (sockfd, (struct sockaddr*) &s_server, sizeof(s_server)) < 0){
  		die ("bind()");
  	}

  get_self_ip (self_addr);

  printf ("== %s : %i ==\n", self_addr, PORT);
  printf ("...waiting for clients...\n");

  //run forever
  for(;;){

//  ret = recvfrom (sockfd, buf, MAX, 0, (struct sockaddr*)&s_client, &slen);  //---eb removed for test
// ---this section was added to recv a test header---
	packet_header header;
  	ret = recvfrom (sockfd, &header, MAX, 0, (struct sockaddr*)&s_client, &slen);			
  	check_for_error (ret, "recvfrom()");

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

// ---end significant changes---

		rw_data.fd = sockfd;
		rw_data.buffer = buf;
		rw_data.client = &s_client;
		rw_data.slen = slen;
		rw_data.ret = ret;

		ret = pthread_create(&t_id, NULL, rw, (void*)&rw_data);
		check_for_error(ret, "pthread_create()");
  	}

  	close (sockfd);
	pthread_exit(NULL);
  	return 0;
}

void get_self_ip (char* addressBuffer){
	struct ifaddrs * ifAddrStruct = NULL;
	struct ifaddrs * ifa = NULL;
   void * tmpAddrPtr = NULL;
   //char addressBuffer[INET_ADDRSTRLEN];

   getifaddrs(&ifAddrStruct);

	for (ifa = ifAddrStruct; ifa != NULL; ifa = ifa->ifa_next) {
   	if (ifa ->ifa_addr->sa_family==AF_INET) {
      	// only IPv4 address
         tmpAddrPtr=&((struct sockaddr_in *)ifa->ifa_addr)->sin_addr;

         if (strcmp(ifa->ifa_name, "en0") == 0
          || strcmp(ifa->ifa_name, "eth0") == 0){
          	inet_ntop(AF_INET, tmpAddrPtr, addressBuffer, INET_ADDRSTRLEN);
            //printf("%s IP Address %s\n", ifa->ifa_name, addressBuffer);
         }
   	}
	}
   if (ifAddrStruct!=NULL)
	freeifaddrs(ifAddrStruct);
}

void *rw(void * data){

	struct t_data* rw_data = (struct t_data*) data;
	int sockfd = rw_data->fd;
	int slen = rw_data->slen;
	int port, ret;
//	char* buf = rw_data->buffer; //---eb removed for test

	struct sockaddr_in* s_client = rw_data->client;
	char ip_addr[INET_ADDRSTRLEN];

	inet_ntop (AF_INET, &(s_client->sin_addr), ip_addr, INET_ADDRSTRLEN);
	port = ntohs (s_client->sin_port);

// ---haven't passed the header to the thread as of yet---
/*
	printf ("==Received packet from %s:%d==\n %s\n",
	ip_addr,
	port,
	buf
	);

	ret = sendto (sockfd, buf, strlen (buf)+1, 0, (struct sockaddr*)s_client, slen);
	check_for_error (ret, "sendto()");
*/
}
