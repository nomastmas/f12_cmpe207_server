#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <errno.h>
#include "connect207.h"
#include "207layer.h"

#if 1 //remove
enum FLAGS {CLOSED, LISTEN, SYN_RCVD, SYN_SENT, ESTABLISHED, FIN_WAIT_1, CLOSE_WAIT, FIN_WAIT_2, CLOSING, LAST_ACK, TIME_WAIT};
	
int TCPStateMachine(int flag, int state);
#endif

#define MAX_BUF_SIZE 256

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
	
	printf("%s: %d: Enter\n",__FUNCTION__,__LINE__);
	switch(tcp_state_in)
	{
		/*Check whether received packet is SYN packet*/	
		case CONNECT207_SYN:
		{	
			printf("%s: %d\n",__FUNCTION__,__LINE__);
			/*No need to check Seq Num, NS flag, CWR flag,ECE flag, Urg Flag, Psh flag, Rst flag */
			
			/*for Ack Num*/
			if(gTcp_Block[tcp_block_index_in].pTcpH->ack_num != 0x00) //Note that for the first SYN in 3-way handshake the Ack Number is set to 0x00 in the first packet. 
			{
				printf("connect207_check_3way_response_packet(): Not SYN cause ACK num=%x\n",gTcp_Block[tcp_block_index_in].pTcpH->ack_num );
				return TCP207_CONNECT_WRONG_3WAY_RESPONSE;
			}


			if(gTcp_Block[tcp_block_index_in].pTcpH->ack_flag != 0x00) //Note that for the first SYN in 3-way handshake the Ack flag should be 0x0. 
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

			printf("@@@@@@ ITS a SYN PACKET ALRIGHT!!!\n");
			break;
		}
		
		case CONNECT207_SYN_ACK: //this is useful for SYN_ACK. 
		{
			/*No need to check Seq Num,NS flag, CWR flag, ECE flag, Urg Flag,Psh flag, Rst flag, Fin flag  */
			
			/*Ack Num*/ 
			if(gTcp_Block[tcp_block_index_in].pTcpH->ack_num != gTcp_Block[tcp_block_index_in].pSeq->send_current_seq_number + 1) //the (received sequencenumber + 1) 
			{
				printf("connect207_check_3way_response_packet(): Not SYNACK cause Ack Num=%x\n",gTcp_Block[tcp_block_index_in].pTcpH->ack_num );
				return TCP207_CONNECT_WRONG_3WAY_RESPONSE;

			}
			
			//Ack Flag
			if(gTcp_Block[tcp_block_index_in].pTcpH->ack_flag != 0x1)
			{
				printf("connect207_check_3way_response_packet(): Not SYNACK cause ACK flag=%x\n",gTcp_Block[tcp_block_index_in].pTcpH->ack_flag );
				return TCP207_CONNECT_WRONG_3WAY_RESPONSE;

			}
			
			//Syn Flag
			if(gTcp_Block[tcp_block_index_in].pTcpH->syn_flag != 0x1) // Set to 1
			{
				printf("connect207_check_3way_response_packet(): not SYNACK cause syn flag=%x\n",gTcp_Block[tcp_block_index_in].pTcpH->syn_flag );
				return TCP207_CONNECT_WRONG_3WAY_RESPONSE;

			}
			printf("@@@@@@@@ ITS a SYNACK PACKET ALRIGHT!!!\n");
			break;

	
		}
		case CONNECT207_ACK: //last step in 3-way handshaking
		{
			/*Ack Num*/ 
			if(gTcp_Block[tcp_block_index_in].pTcpH->ack_num != gTcp_Block[tcp_block_index_in].pSeq->send_current_seq_number + 1) //the (received sequencenumber + 1) 
			{
				printf("connect207_check_3way_response_packet(): Not ACK cause Ack Num=%x\n",gTcp_Block[tcp_block_index_in].pTcpH->ack_num );
				return TCP207_CONNECT_WRONG_3WAY_RESPONSE;

			}
			//Ack Flag
			if(gTcp_Block[tcp_block_index_in].pTcpH->ack_flag != 0x1)
			{
				printf("connect207_check_3way_response_packet(): Not ACK cause ACK flag=%x\n",gTcp_Block[tcp_block_index_in].pTcpH->ack_flag );
				return TCP207_CONNECT_WRONG_3WAY_RESPONSE;

			}
			
			//Syn Flag
			if(gTcp_Block[tcp_block_index_in].pTcpH->syn_flag != 0x0) // Should be 0
			{
				printf("connect207_check_3way_response_packet(): not ACK cause syn flag=%x\n",gTcp_Block[tcp_block_index_in].pTcpH->syn_flag );
				return TCP207_CONNECT_WRONG_3WAY_RESPONSE;

			}
			printf("@@@@@@@@ ITS a ACK PACKET ALRIGHT!!!\n");

			break;
				
		}
		default:
		{
			printf("Default error for connect207_tcp_3way_response_header_fill() %d\n",tcp_state_in);
			return TCP207_CONNECT_WRONG_3WAY_RESPONSE;
			break;	
		}

	}
	
	return TCP207_SUCCESS;

}



/*This function fills up TCP header structure in the global structure, for 3 way handshake depending upon the TCP state machine values for the connect207() function*/
int connect207_tcp_3way_response_header_fill(int tcp_block_index_in, int tcp_state_in  )
{
	int aBufferSize;
	unsigned int aChecksum;
	int retVal = 0;
	unsigned short tmp16=0;
/*Common values to be filled*/
	/*data offset 5 bits*/
	gTcp_Block[tcp_block_index_in].pTcpH->data_offset = 0x14; //20bytes Check ? ???

	/*Window Size 16 bit*/
	gTcp_Block[tcp_block_index_in].pTcpH->window_size = 0x16d0; //5796 Check ? ?

/*Src becomes dest and vice versa*/
	tmp16 = gTcp_Block[tcp_block_index_in].pTcpH->source_port;
	gTcp_Block[tcp_block_index_in].pTcpH->source_port = gTcp_Block[tcp_block_index_in].pTcpH->dest_port ;
	gTcp_Block[tcp_block_index_in].pTcpH->dest_port = tmp16;
	
	switch(tcp_state_in)
	{
		
		case CONNECT207_SYN:
		{	
			/*for Seq Num*/
			gTcp_Block[tcp_block_index_in].pTcpH->seq_num = 0x12345678;//rand(); //seqnum is 32 bit. Initial sequence number value while sending SYN in 3 way handshaking is random value ranginf from 0 to (2^32 -1)  Note that we need to take care to loop the sequence number while going fro, 2^32 -1 to 0

			printf("%s: %d:SYN packet rand=%d\n",__FUNCTION__,__LINE__,gTcp_Block[tcp_block_index_in].pTcpH->seq_num);
			
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
			printf("%s: %d:SYNACK packet rand SeqNum=%d\n",__FUNCTION__,__LINE__,gTcp_Block[tcp_block_index_in].pTcpH->seq_num);
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
			printf("%s: %d:ACK packet SeqNum=%d\n",__FUNCTION__,__LINE__,gTcp_Block[tcp_block_index_in].pTcpH->seq_num);

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
	
#if 0
	/*Checksum calculation before sending*/
	gTcp_Block[tcp_block_index_in].pTcpH->checksum = 0x0; // Check ? ?
	retVal = calculate_checksum((unsigned short *)(gTcp_Block[tcp_block_index_in].pTcpH), sizeof(struct packet_header), aChecksum);
	if(retVal == TCP207_ERROR_NULL_POINTER)
	{
		printf ("Error: calculate_checksum()\n");
		return TCP207_ERROR_NULL_POINTER;
	}

	gTcp_Block[tcp_block_index_in].pTcpH->checksum = aChecksum; 
#endif
#if 1 //For testing and debug
		connect207_print_tcp_header(tcp_block_index_in);
		printf("%s: %d:Exit\n",__FUNCTION__,__LINE__);
#endif
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

	/*IMPORTANT :::::::: for sequence number generation*/
	srand(time(0));
	
	/*3 way handshake procedure begins*/
	/*SYN*/
	printf ("@@@@@@@ SYN packet @@@@@@\n");
	aTcpState = CONNECT207_SYN;
	retVal = connect207_tcp_3way_response_header_fill(tcp_block_index_in, aTcpState);
	if(retVal != TCP207_SUCCESS)
	{
		printf ("Error: sendto() retVal == -1 %s\n",strerror(errno));
		return retVal;	
	}

	/*Send the first SYN packet*/
	retVal = sendto (gTcp_Block[tcp_block_index_in].sockfd, gTcp_Block[tcp_block_index_in].pTcpH , sizeof(struct packet_header), 0, (struct sockaddr*)(gTcp_Block[ tcp_block_index_in].pSocket_info), sizeof(struct sockaddr_in));
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

	retVal = recvfrom (gTcp_Block[tcp_block_index_in].sockfd, buf, MAX_BUF_SIZE, 0, (struct sockaddr*)(gTcp_Block[ tcp_block_index_in].pSocket_info), &slen);
	if(retVal == -1)
	{
		printf ("Error: recvfrom() retVal == -1 %s\n",strerror(errno));
		
	}
	else if(retVal == 0)
	{
		printf ("Error: recfrom() retVal == 0 \n");

	}
	else
	{
		printf ("Success recfrom() retVal= %d\n",retVal);
	}

	retVal = tcp_header_extract_from_recv_packet(tcp_block_index_in, buf);
	if(retVal != TCP207_SUCCESS)
	{
		printf("Error: Returned from connect207_check_3way_retesponse_packet()\n");
		return retVal;
	}


	//Check whther the packet received is correct SYNACK packet IF YES then continue
	retVal = connect207_check_3way_response_packet(tcp_block_index_in,CONNECT207_SYN_ACK);
	if(retVal != TCP207_SUCCESS)
	{
		printf("Error: Returned from connect207_check_3way_retesponse_packet()\n");
		return retVal;
	}

/*SB: Can be put in a loop after understanding TCP state machine*/	
	/*Send ACK*/
	printf ("@@@@@ ACK packet @@@@\n");
	/*Call TCP state machine to send ACK in response to SYN|ACK*/
//	state = SYN_SENT; //SB: After understanding TCPMaxhine code, this state can be taken from current state parameter of TCB Block...
//	flag = 0x1; //SYN
//	retVal = TCPStateMachine(flag, state); 
	//if(retVal == reset)


	/*To be filled by connect207()*/
	aTcpState = CONNECT207_ACK; //SB: Replace with aTcpState = state;
	connect207_tcp_3way_response_header_fill(tcp_block_index_in, aTcpState);
	if(retVal != TCP207_SUCCESS)
	{
		printf ("Error: connect207_tcp_3way_response_header_fill() retVal == -1 %s\n",strerror(errno));
		return retVal;	
	}

	/*Send the first ACK packet*/
	printf ("sendto()\n");
	retVal = sendto (gTcp_Block[tcp_block_index_in].sockfd, gTcp_Block[tcp_block_index_in].pTcpH , sizeof(struct packet_header), 0, (struct sockaddr*)gTcp_Block[ tcp_block_index_in].pSocket_info, slen);
	if(retVal == -1)
	{
		printf ("1111Error: sendto() retVal111 == -1 %s\n",strerror(errno));
		
	}
	else if(retVal == 0)
	{
		printf ("Error: sendto() retVal == 0\n");

	}
	else
	{
		printf ("Success sendto() retVal= %d\n",retVal);
	}

	return TCP207_SUCCESS;

}


/*This function extracts the various fields from the tcp header from the recieved packet and fills all the necessary TCB block variable. It returns error in case of the buffer being empty. Note that this structure manipulated the global variables TCB's pTcpH and pSeq for the recvd... values only
int tcp_block_index_in: index of TCP block to be filled 
pBuffer_in: is the buffer which contains the TCP header from the segment received from the receiver*/
int tcp_header_extract_from_recv_packet(int tcp_block_index_in, char * pBuffer_in )
{
	int i=0;
	unsigned short tmp16=0;
	unsigned int tmp32=0;
#if DEBUG		
		printf("%s:%s: %d: tcp_block_index_in =%d\n",__FILE__,__FUNCTION__,__LINE__, tcp_block_index_in);
#endif
	/*Check for error*/			
	if(pBuffer_in == NULL)
	{
		printf("Error:Input buffer is a Null pointer\n");
		return TCP207_ERROR_NULL_POINTER;
	}

#if 0 //testing Just to fill in values from wirehark SYN
	memset(pBuffer_in, 0, sizeof(struct packet_header));
	printf("%s:%s: %d\n",__FILE__,__FUNCTION__,__LINE__);
	pBuffer_in[0] = 0x00;
	pBuffer_in[1] = 0x26;
	pBuffer_in[2] = 0x62;
	pBuffer_in[3] = 0x2f;
	pBuffer_in[4] = 0x47;
	pBuffer_in[5] = 0x87;
	pBuffer_in[6] = 0x00;
	pBuffer_in[7] = 0x1d;
	pBuffer_in[8] = 0x60;
	pBuffer_in[9] = 0xb3;
	pBuffer_in[10] = 0x01;
	pBuffer_in[11] = 0x84;
	pBuffer_in[12] = 0x08;
	pBuffer_in[13] = 0x00;
	pBuffer_in[14] = 0x45;
	pBuffer_in[15] = 0x00;
	pBuffer_in[16] = 0x00;
	pBuffer_in[17] = 0x3c;
	pBuffer_in[18] = 0x47;
	pBuffer_in[19] = 0x65;
	pBuffer_in[20] = 0x40;

#endif
#if DEBUG
	for(i=0;i<20;i++)
	{
		printf("%dbyte 0x%x,%u: ",i,pBuffer_in[i],pBuffer_in[i]);
	}
	printf("\n");
#endif
	/*Populate TCP header in TCB*/
//	gTcp_Block[tcp_block_index_in].pTcpH  = (struct packet_header *) pBuffer_in;
	tmp16 = (pBuffer_in[0] << 8) | (pBuffer_in[1]);
#if DEBUG	
	printf("%s:%s: %d: tcp_block_index_in =%d\n",__FILE__,__FUNCTION__,__LINE__, tcp_block_index_in);
#endif
	CB[tcp_block_index_in].pTcpH->source_port = ntohs(tmp16);
	
	tmp16 = 0;
	tmp16 = (pBuffer_in[2] << 8) | (pBuffer_in[3]);
	CB[tcp_block_index_in].pTcpH->dest_port = ntohs(tmp16);

	tmp32 = 0; tmp32 = ((pBuffer_in[4]&0xff) << 24) | ((pBuffer_in[5]&0xff) << 16) | ((pBuffer_in[6]&0xff) <<8) | (pBuffer_in[7]&0xff);
	CB[tcp_block_index_in].pTcpH->seq_num = htonl(tmp32);
#if DEBUG	
	printf("%s:%s: %d\n",__FILE__,__FUNCTION__,__LINE__);
#endif
	
	tmp32 = 0; tmp32 = ((pBuffer_in[8]&0xff)<<24) | ((pBuffer_in[9]&0xff) <<16) | ((pBuffer_in[10]&0xff) <<8) | (pBuffer_in[11]&0xff);
	CB[tcp_block_index_in].pTcpH->ack_num = htonl(tmp32);

#if 0
	tmp16 =0; tmp16 = ntohs(((pBuffer_in[12]&0xff)>>3)) ;
	//printf("$$$$$$$$$$$$$$$$$$$$$$$$$$$0x%x\n",tmp16);
	//gTcp_Block[tcp_block_index_in].pTcpH->data_offset= ((pBuffer_in[13]&0xf8)>>3) ;
	gTcp_Block[tcp_block_index_in].pTcpH->data_offset= ((tmp16&0xf800)>>11) ;
	//printf("$$$$$$$$$$$$$$$$0x%x\n",pBuffer_in[13]);
	//printf("$$$$$$$$$$$$$$$$0x%x\n",pBuffer_in[12]);
	gTcp_Block[tcp_block_index_in].pTcpH->reserved = (pBuffer_in[13] & 0x06) >> 1;
#else //SB: CHECK ??????
	tmp32 = ((pBuffer_in[12]&0xff) << 24) | ((pBuffer_in[13]&0xff) << 16) | ((pBuffer_in[14]&0xff) <<8) | (pBuffer_in[15]&0xff);
#if DEBUG
	printf("$$$$$$$$$$$$$$$$0x%x\n",tmp32);
#endif
	//gTcp_Block[tcp_block_index_in].pTcpH->data_offset = ((ntohl(tmp32))&0xff000000) >>24;
	CB[tcp_block_index_in].pTcpH->data_offset = (((tmp32))&0xff000000) >>24;
	
#endif

	CB[tcp_block_index_in].pTcpH->ns_flag = (pBuffer_in[12] & 0x01);

	CB[tcp_block_index_in].pTcpH->cwr_flag = ((pBuffer_in[13]) & 0x01) >> 0;
	CB[tcp_block_index_in].pTcpH->ece_flag = (pBuffer_in[13] & 0x02) >> 1;
	CB[tcp_block_index_in].pTcpH->urg_flag =  (pBuffer_in[13] & 0x04) >> 2;
	CB[tcp_block_index_in].pTcpH->ack_flag = (pBuffer_in[13] & 0x08) >> 3;
	CB[tcp_block_index_in].pTcpH->psh_flag = (pBuffer_in[13] & 0x10) >> 4;
	CB[tcp_block_index_in].pTcpH->rst_flag = (pBuffer_in[13] & 0x20) >> 5;
	CB[tcp_block_index_in].pTcpH->syn_flag = (pBuffer_in[13] & 0x40) >> 6;
	CB[tcp_block_index_in].pTcpH->fin_flag = (pBuffer_in[13] & 0x80) >> 7;


	tmp16 =0; tmp16 = ((pBuffer_in[14]&0xff)<<8) | (pBuffer_in[15]&0xff);
	CB[tcp_block_index_in].pTcpH->window_size	= htons(tmp16);

	tmp16 = 0; tmp16 = ((pBuffer_in[16]&0xff)<<8) | (pBuffer_in[17]&0xff);
	CB[tcp_block_index_in].pTcpH->checksum =  htons(tmp16);

	tmp16 = 0; tmp16 = ((pBuffer_in[18]&0xff)<<8) | (pBuffer_in[19]&0xff);
	CB[tcp_block_index_in].pTcpH->urg_ptr =  htons(tmp16);
#if DEBUG	
		printf("%s:%s: %d\n",__FILE__,__FUNCTION__,__LINE__);
#endif
	/*Fill the sequence number values in the TCB */
	CB[tcp_block_index_in].pSeq->recvd_prev_seq_number = CB[tcp_block_index_in].pSeq->recvd_current_seq_number;
	CB[tcp_block_index_in].pSeq->recvd_current_seq_number = CB[tcp_block_index_in].pTcpH->seq_num;

#if DEBUG //For testing and debug
		printf("%s:%s: %d\n",__FILE__,__FUNCTION__,__LINE__);
#endif


		connect207_print_tcp_header(tcp_block_index_in);
#if DEBUG
		printf("%s:%s: %d\n",__FILE__,__FUNCTION__,__LINE__);
#endif
	return TCP207_SUCCESS;

}

/*Prints the TCP Header*/
void connect207_print_tcp_header(int tcp_block_index_in)
{
	printf("Source Port\t\t%u,0x%x\n", 	CB[tcp_block_index_in].pTcpH->source_port,	CB[tcp_block_index_in].pTcpH->source_port);
	printf("Dest Port\t\t%u,0x%x\n", 	CB[tcp_block_index_in].pTcpH->dest_port, 	CB[tcp_block_index_in].pTcpH->dest_port);
	printf("Seq Num\t\t\t%u,0x%x\n", 	CB[tcp_block_index_in].pTcpH->seq_num, 		CB[tcp_block_index_in].pTcpH->seq_num);
	printf("Ack Num\t\t\t%u,0x%x\n", 	CB[tcp_block_index_in].pTcpH->ack_num, 		CB[tcp_block_index_in].pTcpH->ack_num);
	printf("Data Offset\t\t%u,0x%x\n", 	CB[tcp_block_index_in].pTcpH->data_offset, 	CB[tcp_block_index_in].pTcpH->data_offset);
	printf("Reserved\t\t%u,0x%x\n", 	CB[tcp_block_index_in].pTcpH->reserved, 	CB[tcp_block_index_in].pTcpH->reserved);
	printf("NS Flag\t\t\t%u,0x%x\n", 	CB[tcp_block_index_in].pTcpH->ns_flag, 		CB[tcp_block_index_in].pTcpH->ns_flag);
	printf("CWR Flag\t\t%u,0x%x\n", 	CB[tcp_block_index_in].pTcpH->cwr_flag, 	CB[tcp_block_index_in].pTcpH->cwr_flag);
	printf("ECE Flag\t\t%u,0x%x\n", 	CB[tcp_block_index_in].pTcpH->ece_flag, 	CB[tcp_block_index_in].pTcpH->ece_flag);
	printf("URG Flag\t\t%u,0x%x\n", 	CB[tcp_block_index_in].pTcpH->urg_flag, 	CB[tcp_block_index_in].pTcpH->urg_flag);
	printf("ACK Flag\t\t%u,0x%x\n", 	CB[tcp_block_index_in].pTcpH->ack_flag, 	CB[tcp_block_index_in].pTcpH->ack_flag);
	printf("PSH Flag\t\t%u,0x%x\n", 	CB[tcp_block_index_in].pTcpH->psh_flag, 	CB[tcp_block_index_in].pTcpH->psh_flag);
	printf("RST Flag\t\t%u,0x%x\n", 	CB[tcp_block_index_in].pTcpH->rst_flag, 	CB[tcp_block_index_in].pTcpH->rst_flag);
	printf("SYN Flag\t\t%u,0x%x\n", 	CB[tcp_block_index_in].pTcpH->syn_flag, 	CB[tcp_block_index_in].pTcpH->syn_flag);
	printf("FIN Flag\t\t%u,0x%x\n", 	CB[tcp_block_index_in].pTcpH->fin_flag, 	CB[tcp_block_index_in].pTcpH->fin_flag);
	printf("Window Size\t\t%u,0x%x\n", 	CB[tcp_block_index_in].pTcpH->window_size, 	CB[tcp_block_index_in].pTcpH->window_size);
	printf("Checksum\t\t%u,0x%x\n", 	CB[tcp_block_index_in].pTcpH->checksum, 	CB[tcp_block_index_in].pTcpH->checksum);
	printf("Urgent Ptr\t\t%u,0x%x\n", 	CB[tcp_block_index_in].pTcpH->urg_ptr, 		CB[tcp_block_index_in].pTcpH->urg_ptr);

}
