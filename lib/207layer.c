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
