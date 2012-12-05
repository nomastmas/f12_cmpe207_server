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

//void print_header(struct packet_header *header); 
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
	char recv_buffer[536] = {0};

	int bytes_recv = recv_207(ssockfd, recv_buffer, 576, 0);

	recv_buffer[bytes_recv] = '\0';
	printf("received %d bytes of data\n", bytes_recv);

	strcat(app_buffer, recv_buffer);
	printf("app buffer:\n%s\n\n", app_buffer);

	//increase the window_size after data is taken from the receive buffer into app buffer
	gTcp_Block[ssockfd].window_size = gTcp_Block[ssockfd].window_size + bytes_recv;

	// !!!change the app buffer to uppercase here if we want to demo

	int bytes_sent = send_207(ssockfd, app_buffer, strlen(app_buffer) + 1, 0);
	printf("sent %d bytes back to client\n", bytes_sent); //but check the note on dest address in the function


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
	//check state to make sure that we can be receiving data...ok to receive if ESTABLISHED OR CLOSE WAIT
   if (gTcp_Block[socket].tcp_current_state == 4 || gTcp_Block[socket].tcp_current_state == 6) {
	
		struct sockaddr_in remote_addr;
		socklen_t remote_addr_len = sizeof(remote_addr);

		char packet_in[576];
		memset(packet_in, 0, 576);
		struct packet_header *header_in  = (struct packet_header *) packet_in;

		int rv01 = recvfrom (gTcp_Block[socket].sockfd_udp, packet_in, 556, 0, 
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
}								 //should be zero if only receiving a header; includes NULL is not full packet)
	


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


			//in the send function, not sure if pulling client address correctly from TCB
         //is pSocket info -struct sockaddr_in remote_addr ?;

			ssize_t bytes_sent = sendto (gTcp_Block[socket].sockfd_udp, packet_out, payload_bytes + 20, 0,
										(struct sockaddr*)gTcp_Block[socket].pSocket_info, sizeof(struct sockaddr_in));
		
	}
	return payload_bytes;
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
