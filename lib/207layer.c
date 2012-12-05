#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "207layer.h"


void die (char *s){
	perror(s);
	exit(1);
}

void check_for_error (int ret, char* s){
	if (ret < 0){
		die (s);
	}
}

int get_tcp_state (int tcp_state, struct packet_header recv_header, char* msg){
	switch (tcp_state){
		case CLOSED:
			if (strcmp (msg, "client") == 0){
				//send syn
				tcp_state = SYN_SENT;
			}
			else if (strcmp (msg, "server") == 0){
				tcp_state = LISTEN;
			}
			else {
				tcp_state = LISTEN;
			}
			//return 0;
			break;
		case LISTEN:
			if (recv_header.syn_flag == 1){
				// send syn + ack
				//send_header.syn_flag = 1;
				//send_header.ack_flag = 1;
				// send (sockfd, send_header);
				tcp_state = SYN_RCVD;
			}
			else {
				tcp_state = CLOSED;
			}
		break;

		case SYN_RCVD:
			//printf ("SYN_RCVD\n");
			if (recv_header.ack_flag == 1){
				tcp_state = ESTABLISHED;
			}
			else{
				//loop until recv ack
				tcp_state = CLOSED;
			}
			break;

		case SYN_SENT:
			//work on client later
			if (recv_header.syn_flag == 1 && recv_header.ack_flag ==1 ){
				tcp_state = ESTABLISHED;
			}
			else {
				tcp_state = CLOSED;
			}
			break;
		case ESTABLISHED:
			//printf ("ESTABLISHED\n");
			if (recv_header.fin_flag == 1){
				// send_header.ack_flag = 1;
				// send (sockfd, send_header);
				tcp_state = CLOSE_WAIT;
			}
			else if (strcmp (msg, "close") == 0){
				tcp_state = FIN_WAIT_1;
				//send FIN
			}
			else {
				tcp_state = ESTABLISHED;
			}
			break;
		case FIN_WAIT_1:
			if (recv_header.fin_flag == 1){
				tcp_state = CLOSING;
				//send ack
			}
			else if (recv_header.ack_flag == 1){
				tcp_state = FIN_WAIT_2;
			}
			else {
				tcp_state = FIN_WAIT_1;
			}
			break;
		case CLOSE_WAIT:
			//printf ("CLOSE_WAIT\n");
			// close()
			if (strcmp (msg, "close") == 0){
				tcp_state = LAST_ACK;
				// send (sockfd, send_header);
			}
			else
				tcp_state = CLOSE_WAIT;
			break;
		case FIN_WAIT_2:
			if (recv_header.fin_flag == 1 ){
				tcp_state = TIME_WAIT;
			}
			else {
				tcp_state = FIN_WAIT_2;
			}
			break;
		case CLOSING:
			if (recv_header.ack_flag == 1){
				tcp_state = TIME_WAIT;
			}
			else {
				tcp_state = CLOSING;
			}
			break;		
		case LAST_ACK:
			//printf ("LAST_ACK\n");
			if (recv_header.ack_flag == 1){
				tcp_state = CLOSED;
			}
			else {
				tcp_state = LAST_ACK;
			}
			break;
		case TIME_WAIT:
			sleep (2);
			tcp_state = CLOSED;	
			break;
	}

	return tcp_state;
}

char* get_state_name (int tcp_state){
	const char* state_name[] = {"CLOSED", "LISTEN", "SYN_RCVD", "SYN_SENT", "ESTABLISHED", "FIN_WAIT_1", "CLOSE_WAIT", "FIN_WAIT_2", "CLOSING", "LAST_ACK", "TIME_WAIT"};
	return (char*)state_name[tcp_state];
}

int check_state(int sockfd, char* state){
	return !(strcmp((char*)get_state_name(gTcp_Block[sockfd].tcp_current_state), state));
}

int clear_flags(struct packet_header* packet)
{
	
	packet->ns_flag  = 0;
        packet->cwr_flag = 0;
	packet->ece_flag = 0;
        packet->urg_flag = 0;
        packet->ack_flag = 0;
        packet->psh_flag = 0;
        packet->rst_flag = 0;
        packet->syn_flag = 0;
        packet->fin_flag = 0;
}

void connect207_print_tcp_header2(struct packet_header* pTcpH)
{
	printf("Source Port\t\t%u,0x%x\n", 	pTcpH->source_port,	pTcpH->source_port);
	printf("Dest Port\t\t%u,0x%x\n", 	pTcpH->dest_port, 	pTcpH->dest_port);
	printf("Seq Num\t\t\t%u,0x%x\n", 	pTcpH->seq_num, 	pTcpH->seq_num);
	printf("Ack Num\t\t\t%u,0x%x\n", 	pTcpH->ack_num, 	pTcpH->ack_num);
	printf("Data Offset\t\t%u,0x%x\n", 	pTcpH->data_offset, 	pTcpH->data_offset);
	printf("Reserved\t\t%u,0x%x\n", 	pTcpH->reserved, 	pTcpH->reserved);
	printf("NS Flag\t\t\t%u,0x%x\n", 	pTcpH->ns_flag, 	pTcpH->ns_flag);
	printf("CWR Flag\t\t%u,0x%x\n", 	pTcpH->cwr_flag, 	pTcpH->cwr_flag);
	printf("ECE Flag\t\t%u,0x%x\n", 	pTcpH->ece_flag, 	pTcpH->ece_flag);
	printf("URG Flag\t\t%u,0x%x\n", 	pTcpH->urg_flag, 	pTcpH->urg_flag);
	printf("ACK Flag\t\t%u,0x%x\n", 	pTcpH->ack_flag, 	pTcpH->ack_flag);
	printf("PSH Flag\t\t%u,0x%x\n", 	pTcpH->psh_flag, 	pTcpH->psh_flag);
	printf("RST Flag\t\t%u,0x%x\n", 	pTcpH->rst_flag, 	pTcpH->rst_flag);
	printf("SYN Flag\t\t%u,0x%x\n", 	pTcpH->syn_flag, 	pTcpH->syn_flag);
	printf("FIN Flag\t\t%u,0x%x\n", 	pTcpH->fin_flag, 	pTcpH->fin_flag);
	printf("Window Size\t\t%u,0x%x\n", 	pTcpH->window_size, 	pTcpH->window_size);
	printf("Checksum\t\t%u,0x%x\n", 	pTcpH->checksum, 	pTcpH->checksum);
	printf("Urgent Ptr\t\t%u,0x%x\n", 	pTcpH->urg_ptr, 	pTcpH->urg_ptr);

}

//--------------------------------------
//  recv_207
//--------------------------------------

int recv_207(int socket, char *rcv_buffer, size_t buffer_length, int flags)
{
	int payload_bytes;
	//check state to make sure that we can be receiving data...ok to receive if ESTABLISHED OR CLOSE WAIT
   if (gTcp_Block[socket].tcp_current_state == 4 || gTcp_Block[socket].tcp_current_state == 6) {
	
		struct sockaddr_in remote_addr;
		socklen_t remote_addr_len = sizeof(remote_addr);

		char packet_in[576];
		memset(packet_in, 0, 576);
		struct packet_header *header_in  = (struct packet_header *) packet_in;

		int rv01 = recvfrom (gTcp_Block[socket].sockfd_udp, packet_in, SEGMENT_SIZE, 0, 
								  (struct sockaddr *) &remote_addr, &remote_addr_len);

		int rv02 = tcp_header_extract_from_recv_packet(socket, packet_in);

		//added to TCB; should be filled in when receiving SYN-ACK, but this will do
		gTcp_Block[socket].client_port = gTcp_Block[socket].pTcpH->source_port;

		//if SYN flag is set; send a reset & transition to CLOSE
		if (gTcp_Block[socket].pTcpH->syn_flag == 1){
			gTcp_Block[socket].tcp_current_state = 0;
			printf("received in error: SYN = 1, closing socket\n");
			//send the reset; add later if time
			return;
		}

		//if RST flag is set; transition to CLOSED; send an ack?
		if (gTcp_Block[socket].pTcpH->rst_flag == 1){
			gTcp_Block[socket].tcp_current_state = 0;
			printf("received a reset: RST = 1, closing socket\n");
			return;
		}

		//what do we do if we receive a FIN?

		payload_bytes = rv01 - sizeof(struct packet_header);

#if 1
		if (rv01 == 20)
			printf("received the above 20 byte packet header with no data\n");
		else
			printf("received the above 20 byte packet header and %d bytes of data\n", payload_bytes);
#endif

		if (rv01 > sizeof(struct packet_header)) {
			char *tmp_buffer;
			tmp_buffer = &packet_in[sizeof(struct packet_header)];
			tmp_buffer[rv01 - 20] = '\0';

			strcpy(rcv_buffer, tmp_buffer);
#if 0
			printf("receive buffer:\n%s\n\n", rcv_buffer);
#endif
		}

		//send ACK (should check if we also have data to send, but it's too complicated for now)
		//only if there was data, do we send the ACK

		if (payload_bytes > 0) {

			//adjust window size because these bytes have not been delivered to the application
			gTcp_Block[socket].window_size = gTcp_Block[socket].window_size - payload_bytes;

			char packet_out[576];
			struct packet_header *header_out  = (struct packet_header *) packet_out;
			memset (header_out, 0, sizeof(struct packet_header));

			header_out->data_offset		= 5;
			header_out->source_port 	= gTcp_Block[socket].pTcpH->dest_port;
			header_out->dest_port 		= gTcp_Block[socket].pTcpH->source_port;
			header_out->ack_flag 		= 1;
			header_out->ack_num 			= gTcp_Block[socket].pSeq->recvd_current_seq_number + 1;
			header_out->seq_num 			= gTcp_Block[socket].pSeq->send_current_seq_number; //no data; don't change
			header_out->window_size 	= gTcp_Block[socket].window_size;

			printf("sending an ACK with the following header in response:\n");
			connect207_print_tcp_header2(&packet_out);
			printf("\n");

			ssize_t bytes_sent = sendto (gTcp_Block[socket].sockfd_udp, packet_out, 20, 0,
											(struct sockaddr *) &remote_addr, sizeof(remote_addr));


			//adjust the TCB
			gTcp_Block[socket].pSeq->send_prev_seq_number = gTcp_Block[socket].pSeq->send_current_seq_number;
			//current sequence number stays the same as no data was sent; it's an ACK
		} //end if payload_bytes > 0
	} //end state == ESTABLISHED IF
	return payload_bytes; //this is the data bytes sent, not total bytes (total - 20 bytes header);
}						  	//should be zero if only receiving a header; includes NULL is not full packet)


//--------------------------------------
//  send_207
//--------------------------------------

int send_207(int socket, char *snd_buffer, size_t buffer_length, int flags)
{
	int payload_bytes = 0;
	char packet_in[576];
	memset(packet_in, 0, 576);
	struct packet_header *header_in  = (struct packet_header *) packet_in;

	//check state to make sure that we can be sending data...ok to send if ESTABLISHED
   if (gTcp_Block[socket].tcp_current_state == 4) {
	
			char packet_out[576];
			struct packet_header *header_out  = (struct packet_header *) packet_out;
			memset (header_out, 0, sizeof(struct packet_header));

			header_out->data_offset		= 5;
			header_out->source_port 	= gTcp_Block[socket].cmpe207_port;
			header_out->dest_port 		= gTcp_Block[socket].client_port;
			header_out->ack_flag 		= 1;
			header_out->ack_num 			= gTcp_Block[socket].pSeq->recvd_current_seq_number + 1;
			header_out->seq_num 			= gTcp_Block[socket].pSeq->send_current_seq_number + 1 ; 
			header_out->window_size 	= gTcp_Block[socket].window_size;

			char tmp_buffer[536 + 1];
			strcpy(tmp_buffer, snd_buffer);
			tmp_buffer[536] = '\0';
#if 0
			printf("send buffer:\n%s\n", tmp_buffer);
#endif
			payload_bytes = strlen(tmp_buffer) < 536 ? strlen(tmp_buffer) + 1 : 536;
#if 0
			printf("payload bytes:\n%d\n", payload_bytes);
#endif

			printf("sending %d bytes of data with the following header:\n", payload_bytes);
			connect207_print_tcp_header2(&packet_out);
			printf("\n");

			ssize_t bytes_sent = sendto (gTcp_Block[socket].sockfd_udp, packet_out, payload_bytes + 20, 0,
										(struct sockaddr*)gTcp_Block[socket].pSocket_info, sizeof(struct sockaddr_in));
		
	}
	return payload_bytes;
}
