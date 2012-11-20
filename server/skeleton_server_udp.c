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

Control_Block CB[MAX_SOCKET] = {0};
int cmpe207_port_in_use [MAX_PORT] = {0};

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
//207 socket
	sockfd = cmpe207_socket(CMPE207_FAM, CMPE207_SOC, CMPE207_PROC);
	if (sockfd < 0) {
		die ("socket()");
	}

 	get_self_ip (self_addr);

	memset((char *) &s_server, 0, sizeof(s_server));
	s_server.sin_family = AF_INET;
  	inet_pton(AF_INET, self_addr, &(s_server.sin_addr));
//207 bind	
 	if (cmpe207_bind(sockfd, &s_server, sizeof s_server) < 0){
 		die ("bind()");
 	}
	
	int port = htons(CB[sockfd].sock_struct_UDP->sin_port);
 	printf ("== %s : %i ==\n", self_addr, port);
//207 listen
	cmpe207_listen(sockfd, 10);
 	
	int sockfd_udp = CB[sockfd].sockfd_udp;
	printf ("...waiting for clients...\n");

//207 accept
	int ssockfd = cmpe207_accept(sockfd, &s_server, &slen);
	printf("accept completed \n");
 	//run forever
 	for(;;){

 		ret = recvfrom (sockfd_udp, buf, MAX, 0, (struct sockaddr*)&s_client, &slen);
 		check_for_error (ret, "recvfrom()");

		rw_data.fd = sockfd_udp;
		rw_data.buffer = buf;
		rw_data.client = &s_client;
		rw_data.slen = slen;
		rw_data.ret = ret;

		ret = pthread_create(&t_id, NULL, rw, (void*)&rw_data);
		check_for_error(ret, "pthread_create()");
 	}

 	close (sockfd_udp);
	pthread_exit(NULL);
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

            if (!strcmp(ifa->ifa_name, "en0") == 0 
            	|| !strcmp(ifa->ifa_name, "eth0") == 0){
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
