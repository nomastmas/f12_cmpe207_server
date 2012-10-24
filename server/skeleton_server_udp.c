#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <pthread.h>

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

int main (void){
	printf ("...booting up...\n");

	struct sockaddr_in s_server, s_client;
	int sockfd, ret, slen, t_good;
	char buf[MAX];
	struct t_data rw_data;
	pthread_t t_id;

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

 	printf ("...waiting for clients...\n");
	
 	//run forever
 	for(;;){

 		ret = recvfrom (sockfd, buf, MAX, 0, (struct sockaddr*)&s_client, &slen);
 		check_for_error (ret, "recvfrom()");

		rw_data.fd = sockfd;
		rw_data.buffer = buf;
		rw_data.client = &s_client;
		rw_data.slen = slen;
		rw_data.ret = ret;

		t_good = pthread_create(&t_id, NULL, rw, (void*)&rw_data);
 	}

 	close (sockfd);
	pthread_exit(NULL);
 	return 0;
}
