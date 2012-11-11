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

#include "../lib/207layer.h"

#define	MAX			256

Control_Block CB[MAX_SOCKET] = {0};
int cmpe207_port_in_use [MAX_PORT] = {0};

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
	s_server.sin_family 	 = AF_INET;
	s_server.sin_addr.s_addr = inet_addr (argv[1]);		//inet_addr not recommended
	s_server.sin_port = htons (atoi(argv[2]));

	printf ("...booting up client...\n");
	
	sockfd = cmpe207_socket(CMPE207_FAM, CMPE207_SOC, CMPE207_PROC);
	if (sockfd == -1) {
		die("socket()");
	}
	int sockfd_udp = CB[sockfd].sockfd_udp;

	printf ("send message to UDP echo server:\n");
	fgets (buf, MAX, stdin);
	ret = sendto (sockfd_udp, buf, MAX, 0, (struct sockaddr*)&s_server, slen);
	check_for_error (ret, "sendto()");

	ret = recvfrom (sockfd_udp, buf, MAX, 0, (struct sockaddr*)&s_server, &slen);
	check_for_error (ret, "recvfrom()");

	printf ("==response==\n%s\n", buf);
	printf ("goodbye.\n");

	close (sockfd_udp);
	return 0;
}