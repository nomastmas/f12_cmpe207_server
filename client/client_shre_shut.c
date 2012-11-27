#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <errno.h>
#include "207layer.h"
#include "connect207.h"


#define MAX_BUF_SIZE 256
#define MAX 256
#define SOURCE_PORT 9000


#if 1
int main(int argc, char *argv[] )
{

	int retVal = 0;
	int sockfd, ret, slen;
	char buf[MAX_BUF_SIZE];
	struct addrinfo hints, *servinfo, *p;
	struct sockaddr_in s_server;
	const char *port = argv[2];

	struct sockaddr_in aSocketInfo;
	struct packet_header aTcpH;
	struct sequence aSeq;
	int aIndex=0;
	int aInitiateTeardown= 1;
	int aIsItClient= 1;
	int res=0;

	if ( argc != 3 ){
		printf("usage: client hostname\n");
	}

	
	slen = sizeof(s_server);

	bzero (&s_server,sizeof(s_server));
	s_server.sin_family 	 = AF_INET;
	s_server.sin_addr.s_addr = inet_addr (argv[1]);		//inet_addr not recommended
	//s_server.sin_port		 = htons (atoi(argv[2]));
	s_server.sin_port		 = htons (atoi(argv[2]));


#if 1 /*For connect207() testing*/
	//memset(&(gTcp_Block[aIndex]), 0, sizeof(myTcpBlock));
	memset(&aSocketInfo, 0, sizeof(struct sockaddr_in));
	memset(&aTcpH, 0, sizeof(struct sockaddr_in));
	memset(&aSeq, 0, sizeof(struct sequence));
	
	gTcp_Block[aIndex].pSocket_info = &aSocketInfo;
	gTcp_Block[aIndex].pSocket_info->sin_family = s_server.sin_family; 
	gTcp_Block[aIndex].pSocket_info->sin_addr.s_addr = s_server.sin_addr.s_addr;
	gTcp_Block[aIndex].pSocket_info->sin_port = s_server.sin_port;
	
	gTcp_Block[aIndex].pTcpH = &aTcpH;
	gTcp_Block[aIndex].pTcpH->source_port = SOURCE_PORT;
	gTcp_Block[aIndex].pTcpH->dest_port = ntohs(gTcp_Block[aIndex].pSocket_info->sin_port);
	
	gTcp_Block[aIndex].pSeq = &aSeq;
	

#endif

	printf ("...booting up client...source port:%d,0x%x, dest port=%d,0x%x\n",gTcp_Block[aIndex].pTcpH->source_port,gTcp_Block[aIndex].pTcpH->source_port,gTcp_Block[aIndex].pTcpH->dest_port,gTcp_Block[aIndex].pTcpH->dest_port);
	
	sockfd = socket (AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	if (sockfd == -1) {
		//die("socket()");
		printf("socket()\n");
	}	

	gTcp_Block[aIndex].sockfd_udp = sockfd;


	printf("%s:%s: %d: tcp_block_index_in =%d\n",__FILE__,__FUNCTION__,__LINE__, aIndex);
	aIsItClient = 1;
	retVal= connect207(aIndex, aIsItClient);
	if(retVal != TCP207_SUCCESS)
	{
		printf("connect207() returned with error\n");
		return -1;
	}
	

#if 1 /*For connect207() testing*/
	//memset(&(gTcp_Block[aIndex]), 0, sizeof(myTcpBlock));
	memset(&aSocketInfo, 0, sizeof(struct sockaddr_in));
	memset(&aTcpH, 0, sizeof(struct sockaddr_in));
	memset(&aSeq, 0, sizeof(struct sequence));
	
	gTcp_Block[aIndex].pSocket_info = &aSocketInfo;
	gTcp_Block[aIndex].pSocket_info->sin_family = s_server.sin_family; 
	gTcp_Block[aIndex].pSocket_info->sin_addr.s_addr = s_server.sin_addr.s_addr;
	gTcp_Block[aIndex].pSocket_info->sin_port = s_server.sin_port;
	
	gTcp_Block[aIndex].pTcpH = &aTcpH;
	gTcp_Block[aIndex].pTcpH->source_port = SOURCE_PORT;
	gTcp_Block[aIndex].pTcpH->dest_port = ntohs(gTcp_Block[aIndex].pSocket_info->sin_port);
	
	gTcp_Block[aIndex].pSeq = &aSeq;
	

#endif


	printf("%s:%s: %d: tcp_block_index_in =%d\n",__FILE__,__FUNCTION__,__LINE__, aIndex);
	
	printf("%s:%s: %d: Press 1 to Initiate teardown and 0 to wait for teardow signal\n",__FILE__,__FUNCTION__,__LINE__);
	scanf("%d", &aInitiateTeardown);
	if(aInitiateTeardown)
		printf("Initate Teardown\n");
	else
		printf("Wait for teardown signal\n");
		
	retVal= teardown207(aIndex, aInitiateTeardown);
	if(retVal != TCP207_SUCCESS)
	{
		printf("teardown207() returned with error\n");
		return -1;
	}
	

	//sleep(20);
	printf ("==response==%s\n", buf);
	printf ("goodbye.\n");

	close (sockfd);
	return 0;


}

#endif
