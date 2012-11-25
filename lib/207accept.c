#include "207layer.h"

#include <errno.h>


int cmpe207_accept(int sockfd, struct sockaddr_in *addr, socklen_t * addrlen)
{
//error checking: check sockfd IN USE, in LISTEN state
	if(!gTcp_Block[sockfd].sock_in_use)
		die("accept(): socket not in use.");
	if(!check_state(sockfd, "LISTEN"))
		die("accept(): not in LISTEN state.");

//create slave socket
	int ssockfd;
	char self_addr[INET_ADDRSTRLEN];
	char * str;	
	struct sockaddr_in s_client;

//create slave socket
	ssockfd = cmpe207_socket(CMPE207_FAM, CMPE207_SOC, CMPE207_PROC);
	check_for_error(ssockfd, "accept(): couldn't create slave socket");
	close(gTcp_Block[ssockfd].sockfd_udp);

	gTcp_Block[ssockfd].pSocket_info->sin_family = gTcp_Block[sockfd].pSocket_info->sin_family;
	gTcp_Block[ssockfd].pSocket_info->sin_port = gTcp_Block[sockfd].pSocket_info->sin_port;
	gTcp_Block[ssockfd].pSocket_info->sin_addr = gTcp_Block[sockfd].pSocket_info->sin_addr;
	gTcp_Block[ssockfd].tcp_current_state = gTcp_Block[sockfd].tcp_current_state;
	gTcp_Block[ssockfd].sockfd_udp = gTcp_Block[sockfd].sockfd_udp;
	//printf("fd: %s \n", inet_ntop(AF_INET, &(gTcp_Block[ssockfd].pSocket_info->sin_addr), str, INET_ADDRSTRLEN));

	int rv;
	char buf[MAX_BUF_SIZE];

	int slen = sizeof s_client;
//init packet-header structure
	struct packet_header pTcpH, send_packet;
	gTcp_Block[ssockfd].pTcpH = &pTcpH;
	struct sequence seq = {0};
	gTcp_Block[ssockfd].pSeq = &seq;

//3 way handshake
	printf("--3 way handshake--\n");
	while(!check_state(ssockfd, "ESTABLISHED") && check_state(sockfd, "LISTEN"))	
	{
		rv = recvfrom (gTcp_Block[sockfd].sockfd_udp, buf, MAX_BUF_SIZE, 0, (struct sockaddr*)&s_client, &slen);

		if(rv <= 0)
		{
			printf ("Error: recvfrom() %s\n",strerror(errno));
		}
#if DEBUG
		printf ("tcp_header_extract_from_recv_packet()\n");
#endif
#if DEMO
		printf ("recvd packet header: \n");
#endif
		rv = tcp_header_extract_from_recv_packet(ssockfd, buf);
		if(rv != 0)
		{
			printf("Error: Returned from tcp_header_extract_from_recv_packet()\n");
			return rv;
		}

		gTcp_Block[ssockfd].tcp_current_state = get_tcp_state (gTcp_Block[ssockfd].tcp_current_state, *gTcp_Block[ssockfd].pTcpH, 0);

		bzero (&send_packet, sizeof (send_packet));
		
		send_packet.source_port = gTcp_Block[sockfd].cmpe207_port;
		send_packet.dest_port = gTcp_Block[ssockfd].pTcpH->source_port;

		if(check_state(ssockfd, "SYN_RCVD"))
		{
			gTcp_Block[ssockfd].pSeq->recvd_first_seq_number = gTcp_Block[ssockfd].pSeq->recvd_current_seq_number;
			clear_flags(&send_packet);
			send_packet.syn_flag = 1;
			send_packet.ack_flag = 1;
			send_packet.ack_num = gTcp_Block[ssockfd].pSeq->recvd_current_seq_number + 1;
			send_packet.seq_num = rand();
		}
		else if(check_state(ssockfd, "ESTABLISHED"))
		{
			clear_flags(&send_packet);
			send_packet.ack_flag = 1;
			send_packet.seq_num = gTcp_Block[ssockfd].pTcpH->ack_num;
			send_packet.ack_num = gTcp_Block[ssockfd].pSeq->recvd_current_seq_number + 1;
		}
		else
		{
			printf("accept(): not 3-way handshake packet.");
			continue;
		}
		memcpy (&buf, &send_packet, sizeof (send_packet));
//print send packet
		printf ("\nsent packet header: \n");
		connect207_print_tcp_header2(&send_packet);

		printf("state: %s \n\n", (char*) get_state_name(gTcp_Block[ssockfd].tcp_current_state));

		rv = sendto (gTcp_Block[sockfd].sockfd_udp, buf, MAX_BUF_SIZE, 0, (struct sockaddr*)&s_client, slen);
		check_for_error (rv, "sendto()");

	}

	return ssockfd;
}
/*
printf("state: %s \n", (char*) get_state_name(gTcp_Block[sockfd].tcp_current_state));
	printf("sstate: %s \n", (char*) get_state_name(gTcp_Block[ssockfd].tcp_current_state));
*/

