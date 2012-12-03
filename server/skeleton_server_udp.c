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


int cmpe207_port_in_use [MAX_PORT] = {0};

void get_self_ip (char* addressBuffer);

int main (void){
	printf ("...booting up...\n");

	struct sockaddr_in s_server, s_client;
	int sockfd, ret, slen, t_good;
	char buf[MAX_BUF_SIZE];
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
	
	int port = htons(gTcp_Block[sockfd].pSocket_info->sin_port);
 	printf ("== %s %i ==\n", self_addr, port);
//207 listen
	cmpe207_listen(sockfd, 10);
 	
	int sockfd_udp = gTcp_Block[sockfd].sockfd_udp;

 	//run forever
 	for(;;){
		
		printf ("=====NEW CONNECTION!!!=====\n");
		printf ("...waiting for clients...\n");

//207 accept
		int ssockfd = cmpe207_accept(sockfd, &s_server, &slen);
		printf("accept completed \n");

		printf ("goodbye.\n\n\n\n\n");
 	}

 	close (sockfd_udp);
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

