#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <pthread.h>

#define MAX	512
#define PORT 	9999


void die (char *s){
	perror(s);
	exit(1);
}

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
	struct sockaddr_in* si_other = rw_data->client;
	char ip_addr[INET_ADDRSTRLEN];
	
	inet_ntop (AF_INET, &(si_other->sin_addr), ip_addr, INET_ADDRSTRLEN);
	port = ntohs (si_other->sin_port);

	printf ("==Received packet from %s:%d==\n %s\n", 
			ip_addr,
			port, 
			buf
	);
	
	ret = sendto (sockfd, buf, strlen (buf)+1, 0, (struct sockaddr*)si_other, slen);

	if (ret < 0){
		die ("sendto()");
	}
}

int main (void){
	printf ("...booting up...\n");

	struct sockaddr_in si_me, si_other;
	int sockfd, ret, slen, t_good;
	char buf[MAX];
	struct t_data rw_data;
	pthread_t t_id;

	slen = sizeof(si_other);

	sockfd = socket (AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	if (sockfd < 0) {
		die ("socket()");
	}

	memset((char *) &si_me, 0, sizeof(si_me));
	si_me.sin_family = AF_INET;
 	si_me.sin_port = htons(PORT);
 	si_me.sin_addr.s_addr = htonl(INADDR_ANY);	
 	if (bind (sockfd, (struct sockaddr*) &si_me, sizeof(si_me)) < 0){
 		die ("bind()");
 	}

 	printf ("...waiting for clients...\n");
	
 	//run forever
 	for(;;){
		if(ret = recvfrom (sockfd, buf, MAX, 0, (struct sockaddr*)&si_other, &slen))
		{
			if (ret < 0){
				die ("recvfrom()");
			}

			rw_data.fd = sockfd;
			rw_data.buffer = buf;
			rw_data.client = &si_other;
			rw_data.slen = slen;
			rw_data.ret = ret;

			t_good = pthread_create(&t_id, NULL, rw, (void*)&rw_data);
		}
 		
 	}

 	close (sockfd);
	pthread_exit(NULL);
 	return 0;
}
