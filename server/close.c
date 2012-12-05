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
#include <errno.h>

#include "../lib/207layer.h"
#include "../lib/connect207.h"
#include "../lib/teardown207.h"

struct myTcpBlock gTcp_Block[10];

#define MAX		512
#define SERVER_SOURCE_PORT 	33333

#if 1
struct t_data{
	int fd;
	char* buffer;
	struct sockaddr_in* client;
	int slen;
	int ret;
};
#endif
void get_self_ip (char* addressBuffer);
void* rw (void * data);


int main (void)
{
	int state=0;
	int flag = 0;

	struct sockaddr_in s_server;
	int sockfd, ret, slen, t_good;
	char buf[MAX]={0};
	struct t_data rw_data;
	pthread_t t_id;
	char self_addr[INET_ADDRSTRLEN];
	int retVal=0;
	int aTcpState = 0;
	struct sockaddr_in aSocketInfo;
	struct packet_header aTcpH;
	struct sequence aSeq;
	int aIndex=0;
	char msg[] = "server";
	int aInitiateTeardown = 1; 
	int initiate_in = 0;
	int aTcpStatePacket = 0;
	
	printf ("...booting up server...\n");
	//slen = sizeof(s_client);
	slen = sizeof(s_server);

	/*IMPORTANT :::::::: for sequence number generation*/
	srand(time(0));
	
	sockfd = socket (AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	//sockfd = socket (AF_UNSPEC, SOCK_DGRAM, AI_PASSIVE);
	if (sockfd < 0) {
		//die ("socket()");
		printf ("socket()\n");
	}

	memset((char *) &s_server, 0, sizeof(s_server));
	s_server.sin_family = AF_INET;
	s_server.sin_family = AF_INET;
 	s_server.sin_port = htons(SERVER_SOURCE_PORT);
 	s_server.sin_addr.s_addr = htonl(INADDR_ANY);	
 	if (bind (sockfd, (struct sockaddr*) &s_server, sizeof(s_server)) < 0){
 		//die ("bind()");
 		printf("bind()\n");
 	}

 	get_self_ip (self_addr);

	gTcp_Block[aIndex].pSocket_info = &aSocketInfo;
	gTcp_Block[aIndex].pSocket_info->sin_family = s_server.sin_family; 
	gTcp_Block[aIndex].pSocket_info->sin_addr.s_addr = s_server.sin_addr.s_addr;
	gTcp_Block[aIndex].pSocket_info->sin_port = ntohs(s_server.sin_port);
	
	gTcp_Block[aIndex].pTcpH = &aTcpH;
	gTcp_Block[aIndex].pTcpH->source_port = SERVER_SOURCE_PORT;
	//gTcp_Block[aIndex].pTcpH->dest_port = ; //Client port
	
	gTcp_Block[aIndex].pSeq = &aSeq;

	gTcp_Block[aIndex].sockfd_udp = sockfd;
	printf ("===== %s %i =====\n", self_addr, SERVER_SOURCE_PORT);
 	//printf ("Server Port is : %i, 0x%x \n",SERVER_SOURCE_PORT, SERVER_SOURCE_PORT);
 	//printf ("Src Port is : %i, 0x%x \n",gTcp_Block[aIndex].pTcpH->source_port ,gTcp_Block[aIndex].pTcpH->source_port  );
 	//printf ("sockfd : %i \n",gTcp_Block[aIndex].sockfd_udp );
	//printf ("Dest Port is : %i, 0x%x\n",gTcp_Block[aIndex].pTcpH->dest_port ,gTcp_Block[aIndex].pTcpH->dest_port  );


 	//run forever
 	for(;;){
		printf ("=====LISTENING=====\n");
		printf ("...waiting for clients...\n");

 		//retVal = recvfrom (sockfd, buf, MAX, 0, (struct sockaddr*)&s_client, &slen);
 		retVal = recvfrom (sockfd, buf, MAX, 0, (struct sockaddr*)&s_server, &slen);
		if(retVal == -1)
		{
			printf ("Error: recvfrom() retVal == -1 %s\n",strerror(errno));
			
		}
		else if(retVal == 0)
		{
			printf ("Error: recvfrom() retVal == 0\n");

		}
		else
		{
			//printf ("Success recvfrom() retVal= %d\n",retVal);
			printf ("=====NEW CONNECTION=====\n");
		}
		//printf ("tcp_header_extract_from_recv_packet()\n");
		retVal = tcp_header_extract_from_recv_packet(aIndex, buf);
		if(retVal != 0)
		{
			printf("Error: Returned from tcp_header_extract_from_recv_packet()\n");
			return retVal;
		}

		//printf("%s:%s: %d\n",__FILE__,__FUNCTION__,__LINE__);
		//Check whther the packet received is correct SYN packet IF YES then continue
		//retVal = connect207_check_3way_response_packet(aIndex,CONNECT207_SYN);
		retVal = connect207_check_3way_response_packet(aIndex, SYN_SENT);
		if(retVal != 0)
		{
			printf("Error: Returned from connect207_check_3way_response_packet()\n");
			return retVal;
		}


		/*Send SYNACK*/
		printf ("<<<<<SYN+ACK>>>>>\n");
		/*Call TCP state machine to send SYNACK in response to SYN|ACK*/
		
		aTcpState = LISTEN; //SYNACK
		retVal = get_tcp_state(aTcpState, *(gTcp_Block[aIndex].pTcpH), msg); //SB:The TCP dtate diagram us changing
		aTcpState = retVal;
	
		/*To be filled by connect207() SYNACK*/
		retVal = connect207_tcp_3way_response_header_fill(aIndex, aTcpState);
		if(retVal != 0)
		{
			printf("Error: Returned from connect207_check_3way_response_packet()\n");
			return retVal;
		}

#if 0 //Remove For testing only
	
		connect207_print_tcp_header(aIndex);
#endif
		/*Send the first SYNACK packet*/
		//printf("%s:%s:%d:sendto()\n",__FILE__,__FUNCTION__,__LINE__);
		//retVal = sendto (gTcp_Block[aIndex].sockfd, gTcp_Block[aIndex].pTcpH , sizeof(struct packet_header), 0, (struct sockaddr*)gTcp_Block[ aIndex].pSocket_info, slen);
		retVal = sendto (sockfd, gTcp_Block[aIndex].pTcpH , sizeof(struct packet_header), 0, (struct sockaddr*)&s_server, sizeof(struct sockaddr_in));
		if(retVal == -1)
		{
			printf ("Error: sendto() retVal == -1 %s\n",strerror(errno));
		}
		else if(retVal == 0)
		{
			printf ("Error: sendto() retVal == 0\n");
		}
		else
		{
			printf ("Success sendto() retVal= %d\n",retVal);
		}


		/*Receive ACK*/
		//printf("%s:%s:%d:recvfrom()\n",__FILE__,__FUNCTION__,__LINE__);
		retVal = recvfrom (sockfd, buf, MAX, 0, (struct sockaddr*)&s_server, &slen);
		if(retVal == -1)
		{
			printf ("Error: recvfrom() retVal == -1 %s\n",strerror(errno));
			
		}
		else if(retVal == 0)
		{
			printf ("Error: recvfrom() retVal == 0\n");

		}
		else
		{
			printf ("Success recvfrom() retVal= %d\n",retVal);
		}
		//printf("%s:%s:%d:recvfrom After()\n",__FILE__,__FUNCTION__,__LINE__);

		tcp_header_extract_from_recv_packet(aIndex, buf);

		//Check whther the packet received is correct SYN packet IF YES then continue
		retVal = connect207_check_3way_response_packet(aIndex,ESTABLISHED );//CONNECT207_ACK);
		if(retVal != 0)
		{
			printf("Error: Returned from connect207_check_3way_response_packet()\n");
			return retVal;
		}
		printf("=====ESTABLISHED=====\n");
 	//}
	//printf("%s:%s: %d: Press 1 to Initiate teardown and 0 to wait for teardow signal\n",__FILE__,__FUNCTION__,__LINE__);
	//scanf("%d", &aInitiateTeardown);
		//int recv_207(int socket, char *rcv_buffer, size_t buffer_length, int flags);
		retVal = recv_207(sockfd, buf, MAX, 0);
		printf(">>>>>%s<<<<<", buf);
	aInitiateTeardown = 0;
	if(aInitiateTeardown)
		printf("Initate Teardown\n");
	else
		printf("\n\n\n\n=====TEARDOWN=====\n");
	
#if 0 //remove later
	gTcp_Block[aIndex].pSocket_info = &aSocketInfo;
	gTcp_Block[aIndex].pSocket_info->sin_family = s_server.sin_family; 
	gTcp_Block[aIndex].pSocket_info->sin_addr.s_addr = s_server.sin_addr.s_addr;
	gTcp_Block[aIndex].pSocket_info->sin_port = ntohs(s_server.sin_port);
	
	gTcp_Block[aIndex].pTcpH = &aTcpH;
	gTcp_Block[aIndex].pTcpH->source_port = SERVER_SOURCE_PORT;
	gTcp_Block[aIndex].pTcpH->dest_port = 9999;
	//gTcp_Block[aIndex].pTcpH->dest_port = ; //Client port
	
	gTcp_Block[aIndex].pSeq = &aSeq;

	gTcp_Block[aIndex].sockfd = sockfd;

	retVal= teardown207(aIndex, aInitiateTeardown);
	if(retVal != TCP207_SUCCESS)
	{
		printf("teardown207() returned with error\n");
		return -1;
	}
#else
/*4 way handshake teardown procedure begins*/
	switch (aInitiateTeardown)
	{
		case 1://Initiate shutdown
		{
			/* send FIN */
			/*Call TCP state machine to send FIN*/
			//printf("%s:%s:%d:\n", __FILE__,__FUNCTION__,__LINE__);	
			printf ("@@@@@@@ TCPState=FIN packet\n");
			aTcpStatePacket = FIN207;
			retVal = teardown207_tcp_3way_response_header_fill(aIndex, aTcpStatePacket);
			if(retVal != TCP207_SUCCESS)
			{
				printf ("Error: sendto() retVal == -1 %s\n",strerror(errno));
				return retVal;	
			}

					
		/*Send the packet*/
		retVal = sendto (sockfd, gTcp_Block[aIndex].pTcpH , sizeof(struct packet_header), 0, (struct sockaddr*)&s_server, sizeof(struct sockaddr_in));
		if(retVal == -1)
		{
			printf ("Error: sendto() retVal == -1 %s\n",strerror(errno));
		}
		else if(retVal == 0)
		{
			printf ("Error: sendto() retVal == 0\n");
		}
		else
		{
			printf ("Success sendto() retVal= %d\n",retVal);
		}


	
		retVal = recvfrom (sockfd, buf, MAX, 0, (struct sockaddr*)&s_server, &slen);
		if(retVal == -1)
		{
			printf ("Error: recvfrom() retVal == -1 %s\n",strerror(errno));
			
		}
		else if(retVal == 0)
		{
			printf ("Error: recvfrom() retVal == 0\n");

		}
		else
		{
			printf ("Success recvfrom() retVal= %d\n",retVal);
		}




	
			retVal = tcp_header_extract_from_recv_packet(aIndex, buf);
			if(retVal != TCP207_SUCCESS)
			{
				printf("Error: Returned from teardown207_check_3way_retesponse_packet()\n");
				return retVal;//SB: Handle retransmission ????
			}


			aTcpStatePacket = ACK207;
			retVal = teardown207_check_3way_response_packet(aIndex,aTcpStatePacket);
			if(retVal != TCP207_SUCCESS)
			{
				printf("Error: Returned from teardown207_check_3way_retesponse_packet()\n");
				return retVal;//SB: Handle retransmission 
			}


			/*Now wait for FIN_WAIT_2*/
			//printf("%s:%s:%d:close initiated was complete. Wait for receive \n", __FILE__,__FUNCTION__,__LINE__);	
		

		retVal = recvfrom (sockfd, buf, MAX, 0, (struct sockaddr*)&s_server, &slen);
		if(retVal == -1)
		{
			printf ("Error: recvfrom() retVal == -1 %s\n",strerror(errno));
			
		}
		else if(retVal == 0)
		{
			printf ("Error: recvfrom() retVal == 0\n");

		}
		else
		{
			printf ("Success recvfrom() retVal= %d\n",retVal);
		}

	
			retVal = tcp_header_extract_from_recv_packet(aIndex, buf);
			if(retVal != TCP207_SUCCESS)
			{
				printf("Error: Returned from teardown207_check_3way_retesponse_packet()\n");
				return retVal;//SB: Handle retransmission ????
			}


			aTcpStatePacket = FIN207;
			retVal = teardown207_check_3way_response_packet(aIndex,aTcpStatePacket);
			if(retVal != TCP207_SUCCESS)
			{
				printf("Error: Returned from teardown207_check_3way_retesponse_packet()\n");
				return retVal;//SB: Handle retransmission 
			}

			/*Send ACK207*/
			/*Call TCP state machine to send FIN*/
			printf ("@@@@@@@ TCPState=FIN packet\n");
			aTcpStatePacket = ACK207;
			retVal = teardown207_tcp_3way_response_header_fill(aIndex, aTcpStatePacket);
			if(retVal != TCP207_SUCCESS)
			{
				printf ("Error: sendto() retVal == -1 %s\n",strerror(errno));
				return retVal;	
			}

					
			/*Send the packet*/
			retVal = sendto (sockfd, gTcp_Block[aIndex].pTcpH , sizeof(struct packet_header), 0, (struct sockaddr*)&s_server, sizeof(struct sockaddr_in));
			if(retVal == -1)
			{
				printf ("Error: sendto() retVal == -1 %s\n",strerror(errno));
			}
			else if(retVal == 0)
			{
				printf ("Error: sendto() retVal == 0\n");
			}
			else
			{
				printf ("Success sendto() retVal= %d\n",retVal);
			}


			break;		
			/*Shutdown complete*/	
		}

		case 0://wait for close
		{
			//printf("%s:%s:%d:\n", __FILE__,__FUNCTION__,__LINE__);	
			/*Wait for FIN packet*/	
			retVal = recvfrom (sockfd, buf, MAX, 0, (struct sockaddr*)&s_server, &slen);
			if(retVal == -1)
			{
				printf ("Error: recvfrom() retVal == -1 %s\n",strerror(errno));
				
			}
			else if(retVal == 0)
			{
				printf ("Error: recvfrom() retVal == 0\n");

			}
			else
			{
				printf ("Success recvfrom() retVal= %d\n",retVal);
			}


			retVal = tcp_header_extract_from_recv_packet(aIndex, buf);
			if(retVal != TCP207_SUCCESS)
			{
				printf("Error: Returned from teardown207_check_3way_retesponse_packet()\n");
				return retVal;//SB: Handle retransmission ????
			}

			aTcpStatePacket = FIN207;
			retVal = teardown207_check_3way_response_packet(aIndex,aTcpStatePacket);
			if(retVal != TCP207_SUCCESS)
			{
				printf("Error: Returned from teardown207_check_3way_retesponse_packet()\n");
				return retVal;//SB: Handle retransmission 
			}

			/*Now send ACK207*/
			//printf ("@@@@@@@ TCPState=ACK207 packet\n");
			printf("<<<<<ACK>>>>>\n");
			aTcpStatePacket = ACK207;
			retVal = teardown207_tcp_3way_response_header_fill(aIndex, aTcpStatePacket);
			if(retVal != TCP207_SUCCESS)
			{
				printf ("Error: sendto() retVal == -1 %s\n",strerror(errno));
				return retVal;	
			}

			sleep(2); //Wait for sometime before sending FIN	
			/*Send the packet*/
			retVal = sendto (sockfd, gTcp_Block[aIndex].pTcpH , sizeof(struct packet_header), 0, (struct sockaddr*)&s_server, sizeof(struct sockaddr_in));

			if(retVal == -1)
			{
				printf ("Error: sendto() retVal == -1 %s\n",strerror(errno));
			}
			else if(retVal == 0)
			{
				printf ("Error: sendto() retVal == 0\n");
			}
			else
			{
				printf ("Success sendto() retVal= %d\n",retVal);
			}
			
			/*Now send 207teardown*/
			/* send FIN */
			/*Call TCP state machine to send FIN*/
			//printf ("@@@@@@@ TCPState=FIN packet\n");
			printf("<<<<<FIN>>>>>\n");
			aTcpStatePacket = FIN207;
			retVal = teardown207_tcp_3way_response_header_fill(aIndex, aTcpStatePacket);
			if(retVal != TCP207_SUCCESS)
			{
				printf ("Error: sendto() retVal == -1 %s\n",strerror(errno));
				return retVal;	
			}

						
				/*Send the packet*/
				retVal = sendto (sockfd, gTcp_Block[aIndex].pTcpH , sizeof(struct packet_header), 0, (struct sockaddr*)&s_server, sizeof(struct sockaddr_in));
			if(retVal == -1)
			{
				printf ("Error: sendto() retVal == -1 %s\n",strerror(errno));
			}
			else if(retVal == 0)
			{
				printf ("Error: sendto() retVal == 0\n");
			}
			else
			{
				printf ("Success sendto() retVal= %d\n",retVal);
			}

		

			 retVal = recvfrom (sockfd, buf, MAX, 0, (struct sockaddr*)&s_server, &slen);
			if(retVal == -1)
			{
				printf ("Error: recvfrom() retVal == -1 %s\n",strerror(errno));
			}
			else if(retVal == 0)
			{
				printf ("Error: recvfrom() retVal == 0\n");

			}
			else
			{
				printf ("Success recvfrom() retVal= %d\n",retVal);
			}
		
			retVal = tcp_header_extract_from_recv_packet(aIndex, buf);
			if(retVal != TCP207_SUCCESS)
			{
				printf("Error: Returned from teardown207_check_3way_retesponse_packet()\n");
				return retVal;//SB: Handle retransmission ????
			}
			aTcpStatePacket = ACK207;
			retVal = teardown207_check_3way_response_packet(aIndex,aTcpStatePacket);
			if(retVal != TCP207_SUCCESS)
			{
				printf("Error: Returned from teardown207_check_3way_retesponse_packet()\n");
				return retVal;//SB: Handle retransmission 
			}
			break;
		}
		default:
			//printf("%s:%s:%d:\n", __FILE__,__FUNCTION__,__LINE__);	
			printf("DEFAULT ERROR\n");
			return TCP207_ERROR;

	}

#endif	

		printf ("=====CLOSED=====\n\n\n\n\n");
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
	//	check_for_error (ret, "sendto()");
}
