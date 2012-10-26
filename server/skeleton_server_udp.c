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

#include "../lib/207layer.h"

#define MAX		512
#define PORT 	9999

struct t_data{
	int fd;
	char* buffer;
	struct sockaddr_in* client;
	int slen;
	int ret;
};

void get_self_ip (char* addressBuffer);
void* rw (void * data);

enum FLAGS {CLOSED, LISTEN, SYN_RCVD, SYN_SENT, ESTABLISHED, FIN_WAIT_1, CLOSE_WAIT, FIN_WAIT_2, CLOSING, LAST_ACK, TIME_WAIT};

int main (void){
	printf ("...booting up...\n");

	struct sockaddr_in s_server, s_client;
	int sockfd, ret, slen, tcp_state;
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

	packet_header recv_header, send_header;

	tcp_state = LISTEN;
 	//run forever
 	for(;;){
 		switch (tcp_state){
 			case CLOSED:
 				printf ("CLOSED\n");
 				// placeholder for our close()
 				close (sockfd);
 				return 0;
 				break;
 			case LISTEN:
 				printf ("LISTEN\n");
 				ret = recvfrom (sockfd, buf, MAX, 0, (struct sockaddr*)&s_client, &slen);
 				check_for_error (ret, "recvfrom()");
				// open for connections
 				// listen()
 				//recv_header = buf;
 				memcpy (&recv_header, &buf, sizeof (recv_header));
 				if (recv_header.syn_flag == 1){
 					// send syn + ack
 					send_header.syn_flag = 1;
 					send_header.ack_flag = 1;
 					// send (sockfd, send_header);
 					tcp_state = SYN_RCVD;
 				}
 				else {
 					tcp_state = LISTEN;
 				}

				break;

			case SYN_RCVD:
				printf ("SYN_RCVD\n");
				ret = recvfrom (sockfd, buf, MAX, 0, (struct sockaddr*)&s_client, &slen);
 				check_for_error (ret, "recvfrom()");
 				memcpy (&recv_header, &buf, sizeof (recv_header));
 				if (recv_header.ack_flag == 1){
 					tcp_state = ESTABLISHED;
 				}
 				else{
 					//loop until recv ack
 					tcp_state = SYN_RCVD;
 				}
 				break;

			case SYN_SENT:
				//work on client later
				break;
			case ESTABLISHED:
				printf ("ESTABLISHED\n");
				ret = recvfrom (sockfd, buf, MAX, 0, (struct sockaddr*)&s_client, &slen);
 				check_for_error (ret, "recvfrom()");
 				memcpy (&recv_header, &buf, sizeof (recv_header));
 				if (recv_header.fin_flag == 1){
 					send_header.ack_flag = 1;
 					// send (sockfd, send_header);
 					tcp_state = CLOSE_WAIT;
 				}
				break;
			case FIN_WAIT_1:

				break;
			case CLOSE_WAIT:
				printf ("CLOSE_WAIT\n");
				// close()
				// send (sockfd, send_header);
				tcp_state = LAST_ACK;
				
				break;
			case FIN_WAIT_2:
				
				break;
			case CLOSING:
				
				break;		
			case LAST_ACK:
				printf ("LAST_ACK\n");
				ret = recvfrom (sockfd, buf, MAX, 0, (struct sockaddr*)&s_client, &slen);
 				check_for_error (ret, "recvfrom()");
 				memcpy (&recv_header, &buf, sizeof (recv_header));
 				if (recv_header.ack_flag == 1){
 					tcp_state = CLOSED;
 				}
 				else {
 					tcp_state = LAST_ACK;
 				}
				break;
			case TIME_WAIT:
				break;
 		}
 		//ret = recvfrom (sockfd, buf, MAX, 0, (struct sockaddr*)&s_client, &slen);
 		//check_for_error (ret, "recvfrom()");

 		/*
		rw_data.fd = sockfd;
		rw_data.buffer = buf;
		rw_data.client = &s_client;
		rw_data.slen = slen;
		rw_data.ret = ret;

		ret = pthread_create(&t_id, NULL, rw, (void*)&rw_data);
		check_for_error(ret, "pthread_create()");
		*/
 	}

 	close (sockfd);
	//pthread_exit(NULL);
 	return 0;
}

void get_self_ip (char* addressBuffer){
	struct ifaddrs * ifAddrStruct = NULL;
    struct ifaddrs * ifa  		  = NULL;
    void * tmpAddrPtr 			  = NULL;
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
	char* buf = rw_data->buffer;
	struct sockaddr_in* s_client = rw_data->client;
	char ip_addr[INET_ADDRSTRLEN];
	
	inet_ntop (AF_INET, &(s_client->sin_addr), ip_addr, INET_ADDRSTRLEN);
	port = ntohs (s_client->sin_port);

	printf ("==Received packet from %s:%d==\n %s\n", 
			ip_addr,
			port, 
			buf
	);
	
	ret = sendto (sockfd, buf, strlen (buf)+1, 0, (struct sockaddr*)s_client, slen);
	check_for_error (ret, "sendto()");
}
