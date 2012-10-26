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

int main (int argc, char *argv[]){

	if ( argc != 3 ){
		die("usage: client hostname");
	}

	int sockfd, ret, slen;
	char buf[MAX];
	struct addrinfo hints, *servinfo, *p;
	struct sockaddr_in s_server;
	packet_header send_packet;

	const char *port = argv[2];

	slen = sizeof(s_server);

	bzero (&s_server,sizeof(s_server));
	s_server.sin_family 	 = AF_INET;
	s_server.sin_addr.s_addr = inet_addr (argv[1]);		//inet_addr not recommended
	s_server.sin_port		 = htons (atoi(argv[2]));

	printf ("...booting up client...\n");
	
	sockfd = socket (AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	if (sockfd == -1) {
		die("socket()");
	}
	
	//printf ("send message to UDP echo server:\n");
	//fgets (buf, MAX, stdin);

	printf ("send syn\n");
	send_packet.syn_flag = 1;
	memcpy (&buf, &send_packet, sizeof (send_packet));
	ret = sendto (sockfd, buf, MAX, 0, (struct sockaddr*)&s_server, slen);
	check_for_error (ret, "sendto()");

	printf ("send ack\n");
	bzero (&send_packet, sizeof (send_packet));
	send_packet.ack_flag = 1;
	memcpy (&buf, &send_packet, sizeof (send_packet));
	ret = sendto (sockfd, buf, MAX, 0, (struct sockaddr*)&s_server, slen);
	check_for_error (ret, "sendto()");

	sleep (2);

	printf ("send fin\n");
	bzero (&send_packet, sizeof (send_packet));
	send_packet.fin_flag = 1;
	memcpy (&buf, &send_packet, sizeof (send_packet));
	ret = sendto (sockfd, buf, MAX, 0, (struct sockaddr*)&s_server, slen);
	check_for_error (ret, "sendto()");

	sleep (2);

	printf ("send ack\n");
	bzero (&send_packet, sizeof (send_packet));
	send_packet.ack_flag = 1;
	memcpy (&buf, &send_packet, sizeof (send_packet));
	ret = sendto (sockfd, buf, MAX, 0, (struct sockaddr*)&s_server, slen);
	check_for_error (ret, "sendto()");

	//ret = recvfrom (sockfd, buf, MAX, 0, (struct sockaddr*)&s_server, &slen);
	//check_for_error (ret, "recvfrom()");

	//printf ("==response==\n%s\n", buf);
	printf ("goodbye.\n");

	close (sockfd);
	return 0;
}