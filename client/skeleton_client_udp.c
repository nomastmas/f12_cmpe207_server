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

#define	MAX			256

void die (char *s){
	perror(s);
	exit(1);
}

int main (int argc, char *argv[]){

	if ( argc != 3 ){
		die("usage: client hostname");
	}

	int sockfd, ret, slen;
	char buf[MAX];
	struct addrinfo hints, *servinfo, *p;
	struct sockaddr_in si_other;

	const char *port = argv[2];

	slen = sizeof(si_other);

	bzero(&si_other,sizeof(si_other));
	si_other.sin_family = AF_INET;
	si_other.sin_addr.s_addr=inet_addr(argv[1]);
	si_other.sin_port=htons(atoi(argv[2]));

	printf ("...booting up client...\n");
	
	sockfd = socket (AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	if (sockfd == -1) {
		die("socket()");
	}
	
	printf ("send message to UDP echo server:\n");
	fgets (buf, MAX, stdin);
	ret = sendto (sockfd, buf, MAX, 0, (struct sockaddr*)&si_other, slen);
	if (ret < 0){
		die ("sendto()");
	}
	ret = recvfrom (sockfd, buf, MAX, 0, (struct sockaddr*)&si_other, &slen);
	if (ret < 0){
		die ("recvfrom()");
	}
	printf ("==response==\n%s\n", buf);
	printf ("goodbye.\n");

	close (sockfd);
	return 0;
}