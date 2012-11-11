#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include "connect207.h"
enum FLAGS {CLOSED, LISTEN, SYN_RCVD, SYN_SENT, ESTABLISHED, FIN_WAIT_1, CLOSE_WAIT, FIN_WAIT_2, CLOSING, LAST_ACK, TIME_WAIT};
int TCPStateMachine(int flag, int state);


#define MAX_BUF_SIZE 256
#define MAX 256
#define SOURCE_PORT 8080
 
struct myTcpBlock gTcp_Block[MAX_TCB_SIZE];


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

	if ( argc != 3 ){
		//die("usage: client hostname");
		printf("usage: client hostname\n");
	}


	slen = sizeof(s_server);

	bzero (&s_server,sizeof(s_server));
	s_server.sin_family 	 = AF_INET;
	s_server.sin_addr.s_addr = inet_addr (argv[1]);		//inet_addr not recommended
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
	gTcp_Block[aIndex].pTcpH->dest_port = gTcp_Block[aIndex].pSocket_info->sin_port;
	
	gTcp_Block[aIndex].pSeq = &aSeq;
	

#endif

	printf ("...booting up client...\n");
	
	sockfd = socket (AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	if (sockfd == -1) {
		//die("socket()");
		printf("socket()\n");
	}	

	retVal=connect207(aIndex );
	if(retVal != 0)
	{

		printf("connect207() returned with error\n");
	}
	
	sleep(20);
	printf ("==response==\n%s\n", buf);
	printf ("goodbye.\n");

	close (sockfd);
	return 0;


}

/*Calculates the checksum of the input buffer
unsigned short* pBuffer_in: buffer whose checksum is to be found out 
int buffer_size_in: buffer size
unsigned short checksum_out: value of the calculated checksum*/
int calculate_checksum(unsigned short* pBuffer_in, int buffer_size_in, unsigned short checksum_out)
{
	register int sum = 0;
    	u_short answer = 0;
    	register u_short *w = pBuffer_in;
	register int nleft = buffer_size_in;

	/*Null pointer check*/
	if(pBuffer_in == NULL)
	{
		printf("Error: null buufer \n");
		return TCP207_ERROR_NULL_POINTER;

	}
	
	while (nleft > 1)
	{
		sum += *w++;
		nleft -= 2;
	}
	/* mop up an odd byte, if necessary */
	if (nleft == 1)
	{
		*(u_char *) (&answer) = *(u_char *) w;
		sum += answer;
	}

	/* add back carry outs from top 16 bits to low 16 bits */
	sum = (sum >> 16) + (sum & 0xffff);       /* add hi 16 to low 16 */
	sum += (sum >> 16);               /* add carry */
	answer = ~sum;              /* truncate to 16 bits */
	checksum_out = answer;
	return TCP207_SUCCESS;
}


/*This function will check whether the received TCP packet has the values in response to the 3 way handshake*/
int connect207_check_3way_response_packet(int tcp_block_index_in, int tcp_state_in )
{
	
		
	switch(tcp_state_in)
	{
		/*Check whether received packet is SYN packet*/	
		case CONNECT207_SYN:
		{	
			/*No need to check Seq Num, NS flag, CWR flag,ECE flag, Urg Flag, Psh flag, Rst flag */
			
			/*for Ack Num*/
			if(gTcp_Block[tcp_block_index_in].pTcpH->ack_num != 0x00); //Note that for the first SYN in 3-way handshake the Ack Number is set to 0x00 in the first packet. 
			{
				printf("connect207_check_3way_response_packet(): Not SYN cause ACK num=%x\n",gTcp_Block[tcp_block_index_in].pTcpH->ack_num );
				return TCP207_CONNECT_WRONG_3WAY_RESPONSE;
			}


			if(gTcp_Block[tcp_block_index_in].pTcpH->ack_flag != 0x00); //Note that for the first SYN in 3-way handshake the Ack flag should be 0x0. 
			{
				printf("connect207_check_3way_response_packet(): Not SYN cause ACK flag=%x\n",gTcp_Block[tcp_block_index_in].pTcpH->ack_flag );
				return TCP207_CONNECT_WRONG_3WAY_RESPONSE;

			}	
			

			//Syn Flag
			if(gTcp_Block[tcp_block_index_in].pTcpH->syn_flag != 0x1)
			{
				printf("connect207_check_3way_response_packet(): Not SYN cause SYN flag=%x\n",gTcp_Block[tcp_block_index_in].pTcpH->syn_flag );
				return TCP207_CONNECT_WRONG_3WAY_RESPONSE;

			}


			break;
		}
		
		case CONNECT207_SYN_ACK: //this is useful for SYN_ACK. 
		{
			/*No need to check Seq Num,NS flag, CWR flag, ECE flag, Urg Flag,Psh flag, Rst flag, Fin flag  */
			
			/*Ack Num*/ 
			if(gTcp_Block[tcp_block_index_in].pTcpH->ack_num == gTcp_Block[tcp_block_index_in].pSeq->send_current_seq_number + 1) //the (received sequencenumber + 1) 
			{
				printf("connect207_check_3way_response_packet(): Not SYN cause Ack Num=%x\n",gTcp_Block[tcp_block_index_in].pTcpH->ack_num );
				return TCP207_CONNECT_WRONG_3WAY_RESPONSE;

			}
			
			//Ack Flag
			if(gTcp_Block[tcp_block_index_in].pTcpH->ack_flag == 0x1)
			{
				printf("connect207_check_3way_response_packet(): Not SYN cause ACK flag=%x\n",gTcp_Block[tcp_block_index_in].pTcpH->ack_flag );
				return TCP207_CONNECT_WRONG_3WAY_RESPONSE;

			}
			
			//Syn Flag
			if(gTcp_Block[tcp_block_index_in].pTcpH->syn_flag == 0x1) // Set to 1
			{
				printf("connect207_check_3way_response_packet(): Not SYN cause SYN flag=%x\n",gTcp_Block[tcp_block_index_in].pTcpH->syn_flag );
				return TCP207_CONNECT_WRONG_3WAY_RESPONSE;

			}
			break;

	
		}
		case CONNECT207_ACK: //last step in 3-way handshaking
		{
			/*Seq Num*/
			gTcp_Block[tcp_block_index_in].pSeq->send_prev_seq_number = gTcp_Block[tcp_block_index_in].pSeq->send_current_seq_number;/*Save current to previous*/
			gTcp_Block[tcp_block_index_in].pTcpH->seq_num = (gTcp_Block[tcp_block_index_in].pSeq->send_current_seq_number + 1);  /*assign new seq_num*/

			gTcp_Block[tcp_block_index_in].pSeq->send_current_seq_number = gTcp_Block[tcp_block_index_in].pTcpH->seq_num;  /*save seq_num to current value*/

			/*Ack Num*/ 
			gTcp_Block[tcp_block_index_in].pTcpH->ack_num = gTcp_Block[tcp_block_index_in].pSeq->recvd_current_seq_number + 1; //the (received sequencenumber + 1)
 
			//NS Flag
			gTcp_Block[tcp_block_index_in].pTcpH->ns_flag = 0x0; // Not Set

			//CWR Flag
			gTcp_Block[tcp_block_index_in].pTcpH->cwr_flag = 0x0; // Not Set

			//ECE Flag
			gTcp_Block[tcp_block_index_in].pTcpH->ece_flag = 0x0; // Not Set


			//Urg Flag
			gTcp_Block[tcp_block_index_in].pTcpH->urg_flag = 0x0; // Not Set

			//Ack Flag
			gTcp_Block[tcp_block_index_in].pTcpH->ack_flag = 0x1; // Set to 1
			
			//Psh Flag
			gTcp_Block[tcp_block_index_in].pTcpH->psh_flag = 0x0; // Not Set

			//Rst Flag
			gTcp_Block[tcp_block_index_in].pTcpH->rst_flag = 0x0; // Not Set

			//Syn Flag
			gTcp_Block[tcp_block_index_in].pTcpH->syn_flag = 0x0; // Not Set 

			//Fin Flag
			gTcp_Block[tcp_block_index_in].pTcpH->fin_flag = 0x0; // Not Set

			break;

				
		}
		default:
		{
			printf("Default error for connect207_tcp_3way_response_header_fill() %d\n",tcp_state_in);
			break;	
		}

	}
	


}



/*This function fills up TCP header structure in the global structure, for 3 way handshake depending upon the TCP state machine values for the connect207() function*/
int connect207_tcp_3way_response_header_fill(int tcp_block_index_in, int tcp_state_in  )
{
	int aBufferSize;
	unsigned int aChecksum;
	int retVal = 0;
/*Common values to be filled*/
	/*data offset 5 bits*/
	gTcp_Block[tcp_block_index_in].pTcpH->data_offset = 0x14; //20bytes Check ? ???

	/*Window Size 16 bit*/
	gTcp_Block[tcp_block_index_in].pTcpH->window_size = 0x16d0; //5796 Check ? ?
	
	switch(tcp_state_in)
	{
		
		case CONNECT207_SYN:
		{	
			/*for Seq Num*/
			gTcp_Block[tcp_block_index_in].pTcpH->seq_num = rand(); //seqnum is 32 bit. Initial sequence number value while sending SYN in 3 way handshaking is random value ranginf from 0 to (2^32 -1)  Note that we need to take care to loop the sequence number while going fro, 2^32 -1 to 0
			gTcp_Block[tcp_block_index_in].pSeq->send_first_seq_number = gTcp_Block[tcp_block_index_in].pTcpH->seq_num; /*Save current to firts packet sequence number as it is SYN*/

			gTcp_Block[tcp_block_index_in].pSeq->send_current_seq_number = gTcp_Block[tcp_block_index_in].pTcpH->seq_num;  /*Save current sequence number*/

			
			/*for Ack Num*/
			gTcp_Block[tcp_block_index_in].pTcpH->ack_num = 0x00; //Note that for the first SYN in 3-way handshake the Ack Number is set to 0x00 in the first packet. 


			//NS Flag
			gTcp_Block[tcp_block_index_in].pTcpH->ns_flag = 0x0; // Not Set

			//CWR Flag
			gTcp_Block[tcp_block_index_in].pTcpH->cwr_flag = 0x0; // Not Set

			//ECE Flag
			gTcp_Block[tcp_block_index_in].pTcpH->ece_flag = 0x0; // Not Set

			//Urg Flag
			gTcp_Block[tcp_block_index_in].pTcpH->urg_flag = 0x0; // Not Set

			//Ack Flag
			gTcp_Block[tcp_block_index_in].pTcpH->ack_flag = 0x0; // Not Set
			
			//Psh Flag
			gTcp_Block[tcp_block_index_in].pTcpH->psh_flag = 0x0; // Not Set

			//Rst Flag
			gTcp_Block[tcp_block_index_in].pTcpH->rst_flag = 0x0; // Not Set

			//Syn Flag
			gTcp_Block[tcp_block_index_in].pTcpH->syn_flag = 0x1; // Set to 1

			//Fin Flag
			gTcp_Block[tcp_block_index_in].pTcpH->fin_flag = 0x0; // Not Set

			break;
		}
		
		case CONNECT207_SYN_ACK: //when client sends SYN to server then this will not be useful to send. But for server when it receives SYN it has to send SYN ACK then this is useful. 
		{
			/*Seq Num*/
			gTcp_Block[tcp_block_index_in].pTcpH->seq_num = rand();  /*assign new seq_num as this side has just received a SYN and has to start with a new sequence number.*/

			gTcp_Block[tcp_block_index_in].pSeq->send_current_seq_number = gTcp_Block[tcp_block_index_in].pTcpH->seq_num;  /*save seq_num to current value*/
			
			/*Ack Num*/ 
			gTcp_Block[tcp_block_index_in].pTcpH->ack_num = gTcp_Block[tcp_block_index_in].pSeq->recvd_current_seq_number + 1; //the (received sequencenumber + 1) 


			//NS Flag
			gTcp_Block[tcp_block_index_in].pTcpH->ns_flag = 0x0; // Not Set

			//CWR Flag
			gTcp_Block[tcp_block_index_in].pTcpH->cwr_flag = 0x0; // Not Set

			//ECE Flag
			gTcp_Block[tcp_block_index_in].pTcpH->ece_flag = 0x0; // Not Set


			//Urg Flag
			gTcp_Block[tcp_block_index_in].pTcpH->urg_flag = 0x0; // Not Set

			//Ack Flag
			gTcp_Block[tcp_block_index_in].pTcpH->ack_flag = 0x1; // Set to 1
			
			//Psh Flag
			gTcp_Block[tcp_block_index_in].pTcpH->psh_flag = 0x0; // Not Set

			//Rst Flag
			gTcp_Block[tcp_block_index_in].pTcpH->rst_flag = 0x0; // Not Set

			//Syn Flag
			gTcp_Block[tcp_block_index_in].pTcpH->syn_flag = 0x1; // Set to 1

			//Fin Flag
			gTcp_Block[tcp_block_index_in].pTcpH->fin_flag = 0x0; // Not Set

			break;

	
		}
		case CONNECT207_ACK: //last step in 3-way handshaking
		{
			/*Seq Num*/
			gTcp_Block[tcp_block_index_in].pSeq->send_prev_seq_number = gTcp_Block[tcp_block_index_in].pSeq->send_current_seq_number;/*Save current to previous*/
			gTcp_Block[tcp_block_index_in].pTcpH->seq_num = (gTcp_Block[tcp_block_index_in].pSeq->send_current_seq_number + 1);  /*assign new seq_num*/

			gTcp_Block[tcp_block_index_in].pSeq->send_current_seq_number = gTcp_Block[tcp_block_index_in].pTcpH->seq_num;  /*save seq_num to current value*/

			/*Ack Num*/ 
			gTcp_Block[tcp_block_index_in].pTcpH->ack_num = gTcp_Block[tcp_block_index_in].pSeq->recvd_current_seq_number + 1; //the (received sequencenumber + 1)
 
			//NS Flag
			gTcp_Block[tcp_block_index_in].pTcpH->ns_flag = 0x0; // Not Set

			//CWR Flag
			gTcp_Block[tcp_block_index_in].pTcpH->cwr_flag = 0x0; // Not Set

			//ECE Flag
			gTcp_Block[tcp_block_index_in].pTcpH->ece_flag = 0x0; // Not Set


			//Urg Flag
			gTcp_Block[tcp_block_index_in].pTcpH->urg_flag = 0x0; // Not Set

			//Ack Flag
			gTcp_Block[tcp_block_index_in].pTcpH->ack_flag = 0x1; // Set to 1
			
			//Psh Flag
			gTcp_Block[tcp_block_index_in].pTcpH->psh_flag = 0x0; // Not Set

			//Rst Flag
			gTcp_Block[tcp_block_index_in].pTcpH->rst_flag = 0x0; // Not Set

			//Syn Flag
			gTcp_Block[tcp_block_index_in].pTcpH->syn_flag = 0x0; // Not Set 

			//Fin Flag
			gTcp_Block[tcp_block_index_in].pTcpH->fin_flag = 0x0; // Not Set

			break;

				
		}
		default:
		{
			printf("Default error for connect207_tcp_3way_response_header_fill() %d\n",tcp_state_in);
			break;	
		}

	}
	
	/*Checksum calculation before sending*/
	gTcp_Block[tcp_block_index_in].pTcpH->checksum = 0x0; // Check ? ?

	retVal = calculate_checksum((unsigned short *)(gTcp_Block[tcp_block_index_in].pTcpH), sizeof(struct packet_header), aChecksum);
	if(retVal == TCP207_ERROR_NULL_POINTER)
	{
		printf ("Error: calculate_checksum()\n");
		return TCP207_ERROR_NULL_POINTER;
	}

	gTcp_Block[tcp_block_index_in].pTcpH->checksum = aChecksum; 


	return TCP207_SUCCESS;
	
}
	 

/*connect207() function willestablish an active connection to a remore server. */
int connect207(int tcp_block_index_in )
{
	int retVal = 0;
	int flag = 0;
	int state = 0;
	int aBufferSize=0;
	unsigned int aChecksum=0;
	int aTcpState = 0;
	int slen = 0;
	char buf[256];

	slen = sizeof(struct sockaddr_in);

	/*3 way handshake procedure begins*/
	/*SYN*/
	aTcpState = CONNECT207_SYN;
	connect207_tcp_3way_response_header_fill(tcp_block_index_in, aTcpState);

	/*Send the first SYN packet*/
	printf ("send message to UDP echo server:\n");
	//fgets (buf, MAX, stdin);
	retVal = sendto (gTcp_Block[tcp_block_index_in].sockfd, gTcp_Block[tcp_block_index_in].pTcpH , sizeof(struct packet_header), 0, (struct sockaddr*)&(gTcp_Block[ tcp_block_index_in].pSocket_info), sizeof(struct sockaddr_in));
	//check_for_error (ret, "sendto()");

	retVal = recvfrom (gTcp_Block[tcp_block_index_in].sockfd, buf, MAX, 0, (struct sockaddr*)&(gTcp_Block[ tcp_block_index_in].pSocket_info), &slen);
	//check_for_error (ret, "recvfrom()");

	tcp_header_extract_from_recv_packet(tcp_block_index_in, buf);

	//Check whther the packet received is correct SYNACK packet IF YES then continue
	retVal = connect207_check_3way_response_packet(tcp_block_index_in,CONNECT207_SYN_ACK);
	if(retVal != 0)
	{
		printf("Error: Returned from connect207_check_3way_response_packet()\n");
		return retVal;
	}
	
	/*Send ACK*/
	/*Call TCP state machine to send ACK in response to SYN|ACK*/
	state = SYN_SENT;
	flag = 0x1; //SYN
	retVal = TCPStateMachine(flag, state);
	
	/*To be filled by connect207()*/
	aTcpState = CONNECT207_ACK;
	connect207_tcp_3way_response_header_fill(tcp_block_index_in, aTcpState);


	/*Send the first ACK packet*/
	printf ("send message to UDP echo server:\n");
	//fgets (buf, MAX, stdin);
	retVal = sendto (gTcp_Block[tcp_block_index_in].sockfd, gTcp_Block[tcp_block_index_in].pTcpH , sizeof(struct packet_header), 0, (struct sockaddr*)&gTcp_Block[ tcp_block_index_in].pSocket_info, slen);
//	check_for_error (ret, "sendto()");


}


/*This function extracts the various fields from the tcp header from the recieved packet and fills all the necessary TCB block variable. It returns error in case of the buffer being empty. Note that this structure manipulated the global variables TCB's pTcpH and pSeq for the recvd... values only
int tcp_block_index_in: index of TCP block to be filled 
pBuffer_in: is the buffer which contains the TCP header from the segment received from the receiver*/
int tcp_header_extract_from_recv_packet(int tcp_block_index_in, char * pBuffer_in )
{
	int i=0;
	
	/*Check for error*/			
	if(pBuffer_in == NULL)
	{
		printf("Error:Input buffer is a Null pointer\n");
		return TCP207_ERROR_NULL_POINTER;
	}

#if 1
	for(i=0;i<20;i++)
	{
		pBuffer_in[i] = 0x01;
		pBuffer_in[i+1] = 0x02;
		i++;
	}


	for(i=0;i<20;i++)
	{

		printf("0x%x,%d ",pBuffer_in[i],pBuffer_in[i]);
	}
	printf("\n");

#endif
	/*Populate TCP header in TCB*/
	gTcp_Block[tcp_block_index_in].pTcpH  = (struct packet_header *) pBuffer_in;

	/*Fill the sequence number values in the TCB */
	gTcp_Block[tcp_block_index_in].pSeq->recvd_prev_seq_number = gTcp_Block[tcp_block_index_in].pSeq->recvd_current_seq_number;
	gTcp_Block[tcp_block_index_in].pSeq->recvd_current_seq_number = gTcp_Block[tcp_block_index_in].pTcpH->seq_num;

	
#if 1 //For testing and debug
	printf("Source Port\t\t%d,0x%x\n", 	gTcp_Block[tcp_block_index_in].pTcpH->source_port,gTcp_Block[tcp_block_index_in].pTcpH->source_port);
	printf("Dest Port\t\t%d,0x%x\n", 	gTcp_Block[tcp_block_index_in].pTcpH->dest_port, 	gTcp_Block[tcp_block_index_in].pTcpH->dest_port);
	printf("Seq Num\t\t\t%d,0x%x\n", 	gTcp_Block[tcp_block_index_in].pTcpH->seq_num, 	gTcp_Block[tcp_block_index_in].pTcpH->seq_num);
	printf("Ack Num\t\t\t%d,0x%x\n", 	gTcp_Block[tcp_block_index_in].pTcpH->ack_num, 	gTcp_Block[tcp_block_index_in].pTcpH->ack_num);
	printf("Data Offset\t\t%d,0x%x\n", 	gTcp_Block[tcp_block_index_in].pTcpH->data_offset, 	gTcp_Block[tcp_block_index_in].pTcpH->data_offset);
	printf("Reserved\t\t%d,0x%x\n", 	gTcp_Block[tcp_block_index_in].pTcpH->reserved, 	gTcp_Block[tcp_block_index_in].pTcpH->reserved);
	printf("NS Flag\t\t\t%d,0x%x\n", 	gTcp_Block[tcp_block_index_in].pTcpH->ns_flag, 	gTcp_Block[tcp_block_index_in].pTcpH->ns_flag);
	printf("CWR Flag\t\t%d,0x%x\n", 	gTcp_Block[tcp_block_index_in].pTcpH->cwr_flag, 	gTcp_Block[tcp_block_index_in].pTcpH->cwr_flag);
	printf("ECE Flag\t\t%d,0x%x\n", 	gTcp_Block[tcp_block_index_in].pTcpH->ece_flag, 	gTcp_Block[tcp_block_index_in].pTcpH->ece_flag);
	printf("URG Flag\t\t%d,0x%x\n", 	gTcp_Block[tcp_block_index_in].pTcpH->urg_flag, 	gTcp_Block[tcp_block_index_in].pTcpH->urg_flag);
	printf("ACK Flag\t\t%d,0x%x\n", 	gTcp_Block[tcp_block_index_in].pTcpH->ack_flag, 	gTcp_Block[tcp_block_index_in].pTcpH->ack_flag);
	printf("PSH Flag\t\t%d,0x%x\n", 	gTcp_Block[tcp_block_index_in].pTcpH->psh_flag, 	gTcp_Block[tcp_block_index_in].pTcpH->psh_flag);
	printf("RST Flag\t\t%d,0x%x\n", 	gTcp_Block[tcp_block_index_in].pTcpH->rst_flag, 	gTcp_Block[tcp_block_index_in].pTcpH->rst_flag);
	printf("SYN Flag\t\t%d,0x%x\n", 	gTcp_Block[tcp_block_index_in].pTcpH->syn_flag, 	gTcp_Block[tcp_block_index_in].pTcpH->syn_flag);
	printf("FIN Flag\t\t%d,0x%x\n", 	gTcp_Block[tcp_block_index_in].pTcpH->fin_flag, 	gTcp_Block[tcp_block_index_in].pTcpH->fin_flag);
	printf("Window Size\t\t%d,0x%x\n", 	gTcp_Block[tcp_block_index_in].pTcpH->window_size, 	gTcp_Block[tcp_block_index_in].pTcpH->window_size);
	printf("Checksum\t\t%d,0x%x\n", 	gTcp_Block[tcp_block_index_in].pTcpH->checksum, 	gTcp_Block[tcp_block_index_in].pTcpH->checksum);
	printf("Urgent Ptr\t\t%d,0x%x\n", 	gTcp_Block[tcp_block_index_in].pTcpH->urg_ptr, 	gTcp_Block[tcp_block_index_in].pTcpH->urg_ptr);

#endif
	return TCP207_SUCCESS;

}
