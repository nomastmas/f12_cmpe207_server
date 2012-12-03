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

#include "../lib/207layer.h"

//---> Working between these lines - this is top line

#define DISPLAY			0
#define SEGMENT_SIZE		556

void print_header(struct packet_header *header);

int min (int a, int b) 
{
	return (a <= b ? a : b);
}

int recv_207(int socket, char *rcv_buffer, size_t buffer_length, int flags);
int send_207(int socket, char *snd_buffer, size_t buffer_length, int flags);

//---> Working between these lines -this is bottom line

int cmpe207_port_in_use [MAX_PORT] = {0};

struct t_data{
	int fd;
	char* buffer;
	struct sockaddr_in* client;
	int slen;
	int ret;
};

void get_self_ip (char* addressBuffer);
void* rw (void * data);


int main (void){
	printf ("...booting up...\n");

	struct sockaddr_in s_server, s_client;
	int sockfd, ret, slen, t_good;
	char buf[MAX_BUF_SIZE];
	struct t_data rw_data;
	pthread_t t_id;
	char self_addr[INET_ADDRSTRLEN];

	slen = sizeof(s_client);
//207 socket
	sockfd = cmpe207_socket(CMPE207_FAM, CMPE207_SOC, CMPE207_PROC);
	if (sockfd < 0) {
		die ("socket()");
	}

 	get_self_ip (self_addr);

	memset((char *) &s_server, 0, sizeof(s_server));
	s_server.sin_family = AF_INET;
  	inet_pton(AF_INET, self_addr, &(s_server.sin_addr));
//207 bind	
 	if (cmpe207_bind(sockfd, &s_server, sizeof s_server) < 0){
 		die ("bind()");
 	}
	
	int port = htons(gTcp_Block[sockfd].pSocket_info->sin_port);
 	printf ("== %s %i ==\n", self_addr, port);
//207 listen
	cmpe207_listen(sockfd, 10);
 	
	int sockfd_udp = gTcp_Block[sockfd].sockfd_udp;
	printf ("...waiting for clients...\n");

//207 accept
	int ssockfd = cmpe207_accept(sockfd, &s_server, &slen);
	printf("accept completed \n");

//---> Working between these lines - this is top line

	char app_buffer[8196] = {0};
	char recv_buffer[4096] = {0};

	int bytes_recv = recv_207(ssockfd, recv_buffer, sizeof(recv_buffer), 0);	
	strcat(app_buffer, recv_buffer);
	printf("-->number of app bytes received: %d\n", bytes_recv);

	//increase the window_size after data is taken from the receive buffer into app buffer
	gTcp_Block[ssockfd].window_size = gTcp_Block[ssockfd].window_size + strlen(recv_buffer);

	int test_count = 1;
	while (bytes_recv == (SEGMENT_SIZE - sizeof(struct packet_header))) {
		bytes_recv = recv_207(ssockfd, recv_buffer, sizeof(recv_buffer), 0);
		strcat(app_buffer, recv_buffer);
		gTcp_Block[ssockfd].window_size = gTcp_Block[ssockfd].window_size + strlen(recv_buffer);
		test_count++;
		printf("number of app receives: %d\n", test_count);
		printf("-->number of app bytes received: %d\n", bytes_recv);
	}

// !!!change the app buffer to uppercase here...

	char send_buffer[536];
   int app_bytes = strlen(app_buffer);

	int i;
	int index = 0;
	while (index < app_bytes){
		for (i = 0; i < 536; i++) {
			send_buffer[i] = app_buffer[index];
			index ++;
		}
//		printf("%s\n\n", send_buffer);
      printf("\n");
		int send_size = min(sizeof(send_buffer) + 20, strlen(send_buffer) + 20);
		int bytes_sent = send_207(ssockfd, send_buffer, send_size, 0);
//		printf("sent %d bytes back to client\n", bytes_sent);
		memset(send_buffer, 0, 556);
	} //end while loop


//---> Working between these lines - this is bottom line

/*
 	for(;;){

 		ret = recvfrom (sockfd_udp, buf, MAX_BUF_SIZE, 0, (struct sockaddr*)&s_client, &slen);
 		check_for_error (ret, "recvfrom()");

		rw_data.fd = sockfd_udp;
		rw_data.buffer = buf;
		rw_data.client = &s_client;
		rw_data.slen = slen;
		rw_data.ret = ret;

		ret = pthread_create(&t_id, NULL, rw, (void*)&rw_data);
		check_for_error(ret, "pthread_create()");
 	}
*/
 	close (sockfd_udp);
	pthread_exit(NULL);
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

            if (!strcmp(ifa->ifa_name, "en0") == 0 
            	|| !strcmp(ifa->ifa_name, "eth0") == 0){
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
	
	ret = sendto (sockfd, buf, strlen (buf)+1, 0, (struct sockaddr *)s_client, slen);
	check_for_error (ret, "sendto()");
}

int recv_207(int socket, char *rcv_buffer, size_t buffer_length, int flags)
{
   int payload_bytes;
	char segment_in[SEGMENT_SIZE];
	char segment_out[SEGMENT_SIZE];
	struct packet_header *header_in  = (struct packet_header *) segment_in;
	struct packet_header *header_out  = (struct packet_header *) segment_out;

	//check state to make sure that we can be receiving data...ok to receive if ESTABLISHED OR CLOSE WAIT
   if (gTcp_Block[socket].tcp_current_state == 4 || gTcp_Block[socket].tcp_current_state == 6) {
	
	//	printf("ssockfd:\t%d\n", socket);
	//	printf("sockfd_upd:\t%d\n", gTcp_Block[socket].sockfd_udp);

		struct sockaddr_in cli_addr;
		socklen_t cli_addr_len = sizeof(cli_addr);

		int rv01 = recvfrom (gTcp_Block[socket].sockfd_udp, 
									segment_in, SEGMENT_SIZE, 0, 
								  (struct sockaddr *)&cli_addr, &cli_addr_len);


		//couldn't extract client address from TCB; so added here
		gTcp_Block[socket].client_address = cli_addr;

		struct packet_header pTcpH = {0};
		gTcp_Block[socket].pTcpH = &pTcpH;
		struct sequence seq = {0};
		gTcp_Block[socket].pSeq = &seq;

		printf("received packet header: \n");
		int rv02 = tcp_header_extract_from_recv_packet(socket, segment_in);

		//if SYN flag is set; send a reset & transition to CLOSE
		if (gTcp_Block[socket].pTcpH->syn_flag == 1){
			gTcp_Block[socket].tcp_current_state = 0;
			//send the reset; add later if time
			return;
		}

		//if RST flag is set; transition to CLOSED; send an ack?
		if (gTcp_Block[socket].pTcpH->rst_flag == 1){
			gTcp_Block[socket].tcp_current_state = 0;
			return;
		}

		char *tmp_buffer;
		tmp_buffer = &segment_in[sizeof (struct packet_header)];

		//determine if the segment is all or partial data; all data add null termination
		payload_bytes = min(strlen(tmp_buffer), SEGMENT_SIZE - sizeof(struct packet_header));
		printf("payload was %d bytes:\n", payload_bytes);

		if (payload_bytes == SEGMENT_SIZE - sizeof(struct packet_header)) {
			tmp_buffer[SEGMENT_SIZE - sizeof(struct packet_header)] = '\0'; 
		}
		
		strcpy(rcv_buffer, tmp_buffer);
		//	printf("\n%s\n", rcv_buffer);

		//adjust window size because these bytes have not been delivered to the application
		gTcp_Block[socket].window_size = gTcp_Block[socket].window_size - payload_bytes ;

		//send ACK (should check if we have data to send, but it's too complicated for now)
		//only if there was data...

		if (payload_bytes > 0) {
			struct packet_header header_out;
			memset (&header_out, 0, sizeof(struct packet_header));

			header_out.source_port 	= gTcp_Block[socket].pTcpH->dest_port;
			header_out.dest_port 	= gTcp_Block[socket].pTcpH->source_port;
			header_out.ack_flag 		= 1;
			header_out.ack_num 		= gTcp_Block[socket].pSeq->recvd_current_seq_number + 1;
			header_out.seq_num 		= gTcp_Block[socket].pSeq->send_current_seq_number; //don't change; no data
			header_out.window_size 	= gTcp_Block[socket].window_size;

			printf("sending an ACK with the following header in response:\n");
			connect207_print_tcp_header2(&header_out);
			printf("\n");

			char buffer[20];
			memcpy (&buffer, &header_out, sizeof (header_out));

			int bytes_sent = sendto (gTcp_Block[socket].sockfd_udp, buffer, 20, 0,
											(struct sockaddr*)&cli_addr, cli_addr_len);
		}
	} //end state == ESTABLISHED IF
	return payload_bytes;
}
	
int send_207(int socket, char *snd_buffer, size_t buffer_length, int flags)
{
	int bytes_sent = 0;
	char segment_in[SEGMENT_SIZE];
	char segment_out[SEGMENT_SIZE];
	struct packet_header *header_in  = (struct packet_header *) segment_in;
	struct packet_header *header_out  = (struct packet_header *) segment_out;

	//check state to make sure that we can be receiving data...ok to receive if ESTABLISHED
   if (gTcp_Block[socket].tcp_current_state == 4) {
	
	//	printf("ssockfd:\t%d\n", socket);
	//	printf("sockfd_upd:\t%d\n", gTcp_Block[socket].sockfd_udp);

		struct sockaddr_in cli_addr = gTcp_Block[socket].client_address;
		socklen_t cli_addr_len = sizeof(cli_addr);

		struct packet_header header_out;
		memset (&header_out, 0, sizeof(struct packet_header));

		header_out.source_port 	= gTcp_Block[socket].pTcpH->dest_port;
		header_out.dest_port 	= gTcp_Block[socket].pTcpH->source_port;
		header_out.ack_flag 		= 1;
		header_out.ack_num 		= gTcp_Block[socket].pSeq->recvd_current_seq_number + 1;
		header_out.seq_num 		= gTcp_Block[socket].pSeq->send_current_seq_number + sizeof(snd_buffer); 
		header_out.window_size 	= gTcp_Block[socket].window_size;

		char *payload_out;
		payload_out = &segment_out[sizeof (struct packet_header)];	//position payload in the segment
		strcpy(payload_out, snd_buffer); //copy the buffer into the payload

		printf("sending %d bytes back to client with the following header:\n", strlen(payload_out) + 20);
		connect207_print_tcp_header2(&header_out);

		bytes_sent = sendto (gTcp_Block[socket].sockfd_udp, segment_out, strlen(payload_out) + 20, 0,
										(struct sockaddr*)&cli_addr, cli_addr_len);
		
//		printf("sent %d bytes\n", bytes_sent);
	}
	return bytes_sent;
}

/*
printf("pointer:\t%d\n", gTcp_Block[socket].pSeq);
printf("sequence_number:\t%d\n", gTcp_Block[socket].pTcpH->seq_num);
printf("sfs:\t%d\n", gTcp_Block[socket].pSeq->send_first_seq_number);
printf("scs:\t%d\n", gTcp_Block[socket].pSeq->send_current_seq_number);
printf("sps:\t%d\n", gTcp_Block[socket].pSeq->send_prev_seq_number);
printf("rfs:\t%d\n", gTcp_Block[socket].pSeq->recvd_first_seq_number);
printf("rcs:\t%d\n", gTcp_Block[socket].pSeq->recvd_current_seq_number);
printf("rps:\t%d\n", gTcp_Block[socket].pSeq->recvd_prev_seq_number);

printf("\nsfs:\t%d\n", gTcp_Block[socket].pSeq->send_first_seq_number);
printf("scs:\t%d\n", gTcp_Block[socket].pSeq->send_current_seq_number);
printf("sps:\t%d\n", gTcp_Block[socket].pSeq->send_prev_seq_number);
*/

/*
void print_header(struct packet_header *header)
{
	printf("Source Port\t\t%d\n", 	header->source_port);
	printf("Dest Port\t\t%d\n", 		header->dest_port);
	printf("Seq Num\t\t\t%d\n", 		header->seq_num);
	printf("Ack Num\t\t\t%d\n", 		header->ack_num);
	printf("Data Offset\t\t%d\n", 	header->data_offset);
	printf("Reserved\t\t%d\n", 		header->reserved);
	printf("NS Flag\t\t\t%d\n", 		header->ns_flag);
	printf("CWR Flag\t\t%d\n", 		header->cwr_flag);
	printf("ECE Flag\t\t%d\n", 		header->ece_flag);
	printf("URG Flag\t\t%d\n", 		header->urg_flag);
	printf("ACK Flag\t\t%d\n", 		header->ack_flag);
	printf("PSH Flag\t\t%d\n", 		header->psh_flag);
	printf("RST Flag\t\t%d\n", 		header->rst_flag);
	printf("SYN Flag\t\t%d\n", 		header->syn_flag);
	printf("FIN Flag\t\t%d\n", 		header->fin_flag);
	printf("Window Size\t\t%d\n", 	header->window_size);
	printf("Checksum\t\t%d\n", 		header->checksum);
	printf("Urgent Ptr\t\t%d\n", 		header->urg_ptr);
}
*/
