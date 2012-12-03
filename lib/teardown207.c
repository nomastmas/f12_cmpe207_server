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
#include "teardown207.h"
#include "207layer.h"



#if 0 //remove
enum FLAGS {CLOSED, LISTEN, SYN_RCVD, SYN_SENT, ESTABLISHED, FIN_WAIT_1, CLOSE_WAIT, FIN_WAIT_2, CLOSING, LAST_ACK, TIME_WAIT};

#endif

#define MAX_BUF_SIZE 256


/*This function will check whether the received TCP packet has the values in response to the 3 way handshake*/
int teardown207_check_3way_response_packet(int tcp_block_index_in, int tcp_state_in )
{
	
	//printf("%s: %d: Enter\n",__FUNCTION__,__LINE__);
	switch(tcp_state_in)
	{
		/*Check whether received packet is SYN packet*/	
		case FIN207://TEARDOWN207_FIN:
		{	
			//printf("%s: %d: FIN207\n",__FUNCTION__,__LINE__);
			/*No need to check S flag, CWR flag,ECE flag, Urg Flag, Psh flag, Rst flag */
			
			/*for Ack Num*/
#if 0			// SB: Add it later when we have shutdown in a flow. Rigjt now comment it 
			if(gTcp_Block[tcp_block_index_in].pTcpH->ack_num != gTcp_Block[tcp_block_index_in].pTcpH->send_current_seq_number + 1)    
			{
				printf("teardown207_check_3way_response_packet(): Not FIN cause ACK num=%x\n",gTcp_Block[tcp_block_index_in].pTcpH->ack_num );
				return TCP207_CONNECT_ERROR_WRONG_3WAY_RESPONSE;
			}

			if(gTcp_Block[tcp_block_index_in].pTcpH->ack_flag != 0x01) //Note that for FIN packet ACK is 1. 
			{
				printf("teardown207_check_3way_response_packet(): Not FIN cause ACK flag=%x\n",gTcp_Block[tcp_block_index_in].pTcpH->ack_flag );
				return TCP207_CONNECT_ERROR_WRONG_3WAY_RESPONSE;

			}	
			
#endif

			//FIN flag
			if(gTcp_Block[tcp_block_index_in].pTcpH->fin_flag != 0x1)  //Note that for FIN packet FIN is 1.
			{
				printf("teardown207_check_3way_response_packet(): Not FIN cause FIN flag=%x\n",gTcp_Block[tcp_block_index_in].pTcpH->syn_flag );
				return TCP207_CONNECT_ERROR_WRONG_3WAY_RESPONSE;

			}

			printf(">>>>>FIN<<<<<\n");
			break;
		}
		
		case ACK207://TEARDOWN207_ACK: //last step in 3-way handshaking
		{
			/*Ack Num*/ 
			if(gTcp_Block[tcp_block_index_in].pTcpH->ack_num != gTcp_Block[tcp_block_index_in].pSeq->send_current_seq_number + 1) //the (received sequencenumber + 1) 
			{
				printf("teardown207_check_3way_response_packet(): Not ACK cause Ack Num=%x\n",gTcp_Block[tcp_block_index_in].pTcpH->ack_num );
				return TCP207_CONNECT_ERROR_WRONG_3WAY_RESPONSE;

			}
			//Ack Flag
			if(gTcp_Block[tcp_block_index_in].pTcpH->ack_flag != 0x1)
			{
				printf("teardown207_check_3way_response_packet(): Not ACK cause ACK flag=%x\n",gTcp_Block[tcp_block_index_in].pTcpH->ack_flag );
				return TCP207_CONNECT_ERROR_WRONG_3WAY_RESPONSE;

			}
			
			//Syn Flag
			if(gTcp_Block[tcp_block_index_in].pTcpH->syn_flag != 0x0) // Should be 0
			{
				printf("teardown207_check_3way_response_packet(): not ACK cause syn flag=%x\n",gTcp_Block[tcp_block_index_in].pTcpH->syn_flag );
				return TCP207_CONNECT_ERROR_WRONG_3WAY_RESPONSE;

			}

			//Fin Flag
			if(gTcp_Block[tcp_block_index_in].pTcpH->fin_flag != 0x0) // Should be 0
			{
				printf("teardown207_check_3way_response_packet(): not ACK cause fin flag=%x\n",gTcp_Block[tcp_block_index_in].pTcpH->fin_flag );
				return TCP207_CONNECT_ERROR_WRONG_3WAY_RESPONSE;

			}

			printf(">>>>>ACK<<<<<\n");

			break;
				
		}
		default:
		{
			printf("Default error for teardown207_check_3way_response_packet() %d\n",tcp_state_in);
			return TCP207_CONNECT_ERROR_WRONG_3WAY_RESPONSE;
			break;	
		}

	}
	
	return TCP207_SUCCESS;

}



/*This function fills up TCP header structure in the global structure, for 3 way handshake depending upon the TCP state machine values for the teardown207() function*/
int teardown207_tcp_3way_response_header_fill(int tcp_block_index_in, int tcp_state_in  )
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
	
	printf ("SYN_SENT=%d,SYN_RCVD=%d,ESTABLISHED=%d \n", SYN_SENT, SYN_RCVD,ESTABLISHED);
	switch(tcp_state_in)
	{
		
		case FIN207: //TEARDOWN207_FIN_WAIT_1:
		{	

			/*for Seq Num*/
			gTcp_Block[tcp_block_index_in].pTcpH->seq_num = gTcp_Block[tcp_block_index_in].pSeq->send_current_seq_number + 1;
 + 1; //seqnum is 32 bit. 

			printf("%s: %d:FIN packet seqnum=%d\n",__FUNCTION__,__LINE__,gTcp_Block[tcp_block_index_in].pTcpH->seq_num);
			
			gTcp_Block[tcp_block_index_in].pSeq->send_current_seq_number = gTcp_Block[tcp_block_index_in].pTcpH->seq_num;  /*Save current sequence number*/

			
			/*for Ack Num*/
			gTcp_Block[tcp_block_index_in].pTcpH->ack_num = gTcp_Block[tcp_block_index_in].pSeq->recvd_current_seq_number; //Note that for the first SYN in 3-way handshake the Ack Number is set to 0x00 in the first packet. 


			//NS Flag
			gTcp_Block[tcp_block_index_in].pTcpH->ns_flag = 0x0; // Not Set

			//CWR Flag
			gTcp_Block[tcp_block_index_in].pTcpH->cwr_flag = 0x0; // Not Set

			//ECE Flag
			gTcp_Block[tcp_block_index_in].pTcpH->ece_flag = 0x0; // Not Set

			//Urg Flag
			gTcp_Block[tcp_block_index_in].pTcpH->urg_flag = 0x0; // Not Set

			//Ack Flag
			//gTcp_Block[tcp_block_index_in].pTcpH->ack_flag = 0x1; // Set ACK flag 1
			gTcp_Block[tcp_block_index_in].pTcpH->ack_flag = 0x0; // Set ACK flag 0
			
			//Psh Flag
			gTcp_Block[tcp_block_index_in].pTcpH->psh_flag = 0x0; // Not Set

			//Rst Flag
			gTcp_Block[tcp_block_index_in].pTcpH->rst_flag = 0x0; // Not Set

			//Syn Flag
			gTcp_Block[tcp_block_index_in].pTcpH->syn_flag = 0x0; // Not Set 

			//Fin Flag
			gTcp_Block[tcp_block_index_in].pTcpH->fin_flag = 0x1; // Set to 1 

			break;
		}
		
		
		case ACK207://TEARDOWN207_ACK: //last step in 3-way handshaking
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
			printf("Default error for teardown207_tcp_3way_response_header_fill() %d\n",tcp_state_in);
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
	 

/*teardown207() function will establish an active connection to a remore server. */
int teardown207(int tcp_block_index_in, int initiate_in )
{
	int retVal = 0;
	int retValS = 0;
	int retValS1 = 0;
	int flag = 0;
//	int state = 0;
	int aBufferSize=0;
	unsigned int aChecksum=0;
	int aTcpState = 0;
	int slen = 0;
	char buf[256];
	char msg[10]; 
	int aExit = 0;
	int aTcpStatePacket = 0;
	char msg1[10]; 
	
	slen = sizeof(struct sockaddr_in);

	/*IMPORTANT :::::::: for sequence number generation*/
	srand(time(0));
	

	if(initiate_in)//initiate shutdown
	{
		aTcpState = ESTABLISHED;
 
	}
	else//wait for other side to shutdown
	{
		aTcpState = FIN_WAIT_1 ; 
		
	}


 	printf ("sockfd : %i \n",gTcp_Block[tcp_block_index_in].sockfd_udp  );
	/*4 way handshake teardown procedure begins*/
	switch (initiate_in)
	{
		case 1://Initiate shutdown
		{
			/* send FIN */
			/*Call TCP state machine to send FIN*/
			printf("%s:%s:%d:\n", __FILE__,__FUNCTION__,__LINE__);	
			printf ("@@@@@@@ TCPState=FIN packet\n");
			aTcpStatePacket = FIN207;
			retVal = teardown207_tcp_3way_response_header_fill(tcp_block_index_in, aTcpStatePacket);
			if(retVal != TCP207_SUCCESS)
			{
				printf ("Error: sendto() retVal == -1 %s\n",strerror(errno));
				return retVal;	
			}

					
			/*Send the packet*/
			retVal = sendto (gTcp_Block[tcp_block_index_in].sockfd_udp, gTcp_Block[tcp_block_index_in].pTcpH , sizeof(struct packet_header), 0, (struct sockaddr*)(gTcp_Block[ tcp_block_index_in].pSocket_info), sizeof(struct sockaddr_in));
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

			retVal = recvfrom (gTcp_Block[tcp_block_index_in].sockfd_udp, buf, MAX_BUF_SIZE, 0, (struct sockaddr*)(gTcp_Block[ tcp_block_index_in].pSocket_info), &slen); //SB: Handle retarnsmission by using timeout
			if(retVal == -1)
			{
				printf ("Error: recvfrom() retVal == -1 %s\n",strerror(errno)); //SB: Handle retransmission
				
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
				printf("Error: Returned from teardown207_check_3way_retesponse_packet()\n");
				return retVal;//SB: Handle retransmission ????
			}


			aTcpStatePacket = ACK207;
			retVal = teardown207_check_3way_response_packet(tcp_block_index_in,aTcpStatePacket);
			if(retVal != TCP207_SUCCESS)
			{
				printf("Error: Returned from teardown207_check_3way_retesponse_packet()\n");
				return retVal;//SB: Handle retransmission 
			}


			/*Now wait for FIN_WAIT_2*/
			printf("%s:%s:%d:close initiated was complete. Wait for receive \n", __FILE__,__FUNCTION__,__LINE__);	
			retVal = recvfrom (gTcp_Block[tcp_block_index_in].sockfd_udp, buf, MAX_BUF_SIZE, 0, (struct sockaddr*)(gTcp_Block[ tcp_block_index_in].pSocket_info), &slen); //SB: Handle retarnsmission by usiating timeout
			if(retVal == -1)
			{
				printf ("Error: recvfrom() retVal == -1 %s\n",strerror(errno)); //SB: Handle retransmission
				
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
				printf("Error: Returned from teardown207_check_3way_retesponse_packet()\n");
				return retVal;//SB: Handle retransmission ????
			}


			aTcpStatePacket = FIN207;
			retVal = teardown207_check_3way_response_packet(tcp_block_index_in,aTcpStatePacket);
			if(retVal != TCP207_SUCCESS)
			{
				printf("Error: Returned from teardown207_check_3way_retesponse_packet()\n");
				return retVal;//SB: Handle retransmission 
			}

			/*Send ACK207*/
			/*Call TCP state machine to send FIN*/
			printf ("@@@@@@@ TCPState=FIN packet\n");
			aTcpStatePacket = ACK207;
			retVal = teardown207_tcp_3way_response_header_fill(tcp_block_index_in, aTcpStatePacket);
			if(retVal != TCP207_SUCCESS)
			{
				printf ("Error: sendto() retVal == -1 %s\n",strerror(errno));
				return retVal;	
			}

					
			/*Send the packet*/
			retVal = sendto (gTcp_Block[tcp_block_index_in].sockfd_udp, gTcp_Block[tcp_block_index_in].pTcpH , sizeof(struct packet_header), 0, (struct sockaddr*)(gTcp_Block[ tcp_block_index_in].pSocket_info), sizeof(struct sockaddr_in));
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
			/*Waait for FIN packet*/	
			retVal = recvfrom (gTcp_Block[tcp_block_index_in].sockfd_udp, buf, MAX_BUF_SIZE, 0, (struct sockaddr*)(gTcp_Block[ tcp_block_index_in].pSocket_info), &slen); //SB: Handle retarnsmission by using timeout
			if(retVal == -1)
			{
				printf ("Error: recvfrom() retVal == -1 %s\n",strerror(errno)); //SB: Handle retransmission
				
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
				printf("Error: Returned from teardown207_check_3way_retesponse_packet()\n");
				return retVal;//SB: Handle retransmission ????
			}

			aTcpStatePacket = FIN207;
			retVal = teardown207_check_3way_response_packet(tcp_block_index_in,aTcpStatePacket);
			if(retVal != TCP207_SUCCESS)
			{
				printf("Error: Returned from teardown207_check_3way_retesponse_packet()\n");
				return retVal;//SB: Handle retransmission 
			}

			/*Now send ACK207*/
			printf ("@@@@@@@ TCPState=ACK207 packet\n");
			aTcpStatePacket = ACK207;
			retVal = teardown207_tcp_3way_response_header_fill(tcp_block_index_in, aTcpStatePacket);
			if(retVal != TCP207_SUCCESS)
			{
				printf ("Error: sendto() retVal == -1 %s\n",strerror(errno));
				return retVal;	
			}

			sleep(2); //Wait for sometime before sending FIN	
			/*Send the packet*/
			retVal = sendto (gTcp_Block[tcp_block_index_in].sockfd_udp, gTcp_Block[tcp_block_index_in].pTcpH , sizeof(struct packet_header), 0, (struct sockaddr*)(gTcp_Block[ tcp_block_index_in].pSocket_info), sizeof(struct sockaddr_in));
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
			printf ("@@@@@@@ TCPState=FIN packet\n");
			aTcpStatePacket = FIN207;
			retVal = teardown207_tcp_3way_response_header_fill(tcp_block_index_in, aTcpStatePacket);
			if(retVal != TCP207_SUCCESS)
			{
				printf ("Error: sendto() retVal == -1 %s\n",strerror(errno));
				return retVal;	
			}

					
			/*Send the packet*/
			retVal = sendto (gTcp_Block[tcp_block_index_in].sockfd_udp, gTcp_Block[tcp_block_index_in].pTcpH , sizeof(struct packet_header), 0, (struct sockaddr*)(gTcp_Block[ tcp_block_index_in].pSocket_info), sizeof(struct sockaddr_in));
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

			retVal = recvfrom (gTcp_Block[tcp_block_index_in].sockfd_udp, buf, MAX_BUF_SIZE, 0, (struct sockaddr*)(gTcp_Block[ tcp_block_index_in].pSocket_info), &slen); //SB: Handle retarnsmission by using timeout
			if(retVal == -1)
			{
				printf ("Error: recvfrom() retVal == -1 %s\n",strerror(errno)); //SB: Handle retransmission
				
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
				printf("Error: Returned from teardown207_check_3way_retesponse_packet()\n");
				return retVal;//SB: Handle retransmission ????
			}
			aTcpStatePacket = ACK207;
			retVal = teardown207_check_3way_response_packet(tcp_block_index_in,aTcpStatePacket);
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
	printf("EXIT() teardown207 with success\n");
	return TCP207_SUCCESS;

}





