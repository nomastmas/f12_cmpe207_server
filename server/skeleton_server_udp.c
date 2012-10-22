#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <netinet/in.h>

#define MAX		512
#define PORT 	9999

void die (char *s){
	perror(s);
	exit(1);
}

int main (void){
	struct sockadder_in si_me, si_other;
	int sockfd, i, ret, slen;
	char buf[MAX];

	slen = sizeof(si_other);

	sockfd = socket (AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	if (sockfd < 0) {
		die ("socket()");
	}

	memset((char *) &si_me, 0, sizeof(si_me));
	si_me.sin_family = AF_INET;
 	si_me.sin_port = htons(*port);
 	si_me.sin_addr.s_addr = htonl(INADDR_ANY);	
 	if (bind (httpd, (struct sockaddr*) &si_me, sizeof(si_me)) < 0){
 		die ("bind()");
 	}

 	//run forever
 	for(;;){
 		ret = recvfrom (sockfd, buf, MAX, 0, &si_other, &slen);
 		if (ret < 0){
 			die ("recvfrom()");
 		}
 		printf ("==Received packet from %s:%d==\n%s\n", 
 				inet_ntop (si_other.sin_addr), ntosh (si_other.sin_port), buf);

 		sendto (sockfd, buf, strlen (buf)+1, 0, (struct sockadder*) &si_other, slen);
 	}

 	close (sockfd);
 	return 0;
}