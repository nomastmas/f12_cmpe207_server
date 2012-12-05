#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <netinet/in.h>

#include <pthread.h>
#include <unistd.h>

#include <time.h>
#include <sys/time.h>

#define APP_BUF_MAX 8192

//typedef union {
//    char full_20_byte[20]; 
    // assuming little endian other byte order is swapped
    struct tcp_header {
        unsigned short int source_port;
        unsigned short int dest_port;
        unsigned int seq_num;
        unsigned int ack_num;
        unsigned int data_offset : 4;
        unsigned int reserved : 3;
        unsigned int ns_flag :  1;
        unsigned int cwr_flag : 1;
        unsigned int ece_flag : 1;
        unsigned int urg_flag : 1;
        unsigned int ack_flag : 1;
        unsigned int psh_flag : 1;
        unsigned int rst_flag : 1;
        unsigned int syn_flag : 1;
        unsigned int fin_flag : 1;
        unsigned short int window_size;
        unsigned short int checksum;
        unsigned short int urg_ptr;
    }__attribute__((packed));
//} packet_header;

char segment[556];
struct tcp_header *header = (struct tcp_header *) segment;
void print_header(struct tcp_header *header);

//----------------------------------------------------------
//                TCP Control Block
//----------------------------------------------------------

struct tcp_control_block {
	int sd;
	struct sockaddr_in dest_addr;
};

struct tcp_control_block server_tcb[10] = {0};

//----------------------------------------------------------
//              Fuctions - back into header file
//----------------------------------------------------------

int min(int a, int b) {
return (a < b ? a : b);
}

void die (char *s){
	perror(s);
	exit(1);
}

void check_for_error(int ret, char* s){
	if (ret < 0){
		die (s);
	}
}

void print_header (struct tcp_header *header);
int send_207(int sockfd, const unsigned char *buffer, uint32_t buffer_size, int flags);
void zero_header (struct tcp_header *header);

//----------------------------------------------------------
//                  Program Main
//----------------------------------------------------------

int main (int argc, char *argv[]){

//----------------------------------------------------------
//            Initialize the Header for SYN
//----------------------------------------------------------
	header->source_port 	= 2222;
	header->dest_port 	= 0;
	header->seq_num 		= 1000;
	header->ack_num 		= 0;
	header->data_offset 	= 5; 
	header->reserved 		= 0;
	header->ns_flag 		= 0;
	header->cwr_flag 		= 0;
	header->ece_flag 		= 0;
	header->urg_flag 		= 0;
	header->ack_flag 		= 0;
	header->psh_flag 		= 0;
	header->rst_flag 		= 0;
	header->syn_flag 		= 1;
	header->fin_flag 		= 0;
	header->window_size 	= 4096;
	header->checksum 		= 0;
	header->urg_ptr 		= 0;

//print_header (header);

	if ( argc != 3 ){
		die("usage: client hostname");
	}

	int sockfd;
	struct sockaddr_in server_address;
	socklen_t saddr_len = sizeof(server_address);

	bzero (&server_address,sizeof(server_address));
	server_address.sin_family = AF_INET;
	server_address.sin_addr.s_addr = inet_addr (argv[1]);	//inet_addr not recommended
	server_address.sin_port	= htons (atoi(argv[2]));

	sockfd = socket (AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	if (sockfd == -1) {
		die("socket()");
	}

//this gets filled in earlier....
	server_tcb[0].sd = sockfd;
	server_tcb[0].dest_addr = server_address;

// '3' is hard-coded; not sure how to input the socket descriptor for the correct underlying
// upd socket; need error checking, probably in the send_207 function
//	int return_value = send_207(3, send_buffer, sizeof(send_buffer), 0); 

//----------------------------------------------------------
//                     Send the SYN (1)
//----------------------------------------------------------

	ssize_t bytes_sent = sendto(sockfd, header, 20, 0, 
										(struct sockaddr *) &server_address, sizeof(server_address));
			  
	printf("sent SYN: %d bytes using header:\n", bytes_sent);
	print_header(header);
	printf("\n");

//----------------------------------------------------------
//                     RECEIVE SYN-ACK (01)
//----------------------------------------------------------

	char buffer[1024];
   int num_bytes_received = recvfrom(sockfd, segment, 1024, 0, (struct sockaddr *) &server_address, &saddr_len);
	printf("\nreceived %d bytes with header:\n", num_bytes_received);
	print_header(header);
	printf("\n");
//----------------------------------------------------------
//            Initialize the Header for ACK
//----------------------------------------------------------

	header->dest_port 	= header->source_port;
	header->source_port 	= 2222;
	header->dest_port 	= 9999;
	header->seq_num 		= 1001;
	header->ack_num 		= 101;
	header->data_offset 	= 5; 
	header->reserved 		= 0;
	header->ns_flag 		= 0;
	header->cwr_flag 		= 0;
	header->ece_flag 		= 0;
	header->urg_flag 		= 0;
	header->ack_flag 		= 1;
	header->psh_flag 		= 0;
	header->rst_flag 		= 0;
	header->syn_flag 		= 0;
	header->fin_flag 		= 0;
	header->window_size 	= 4096;
	header->checksum 		= 0;
	header->urg_ptr 		= 0;


//----------------------------------------------------------
//                     Send the ACK (2)
//----------------------------------------------------------
	bytes_sent = sendto(sockfd, header, 20, 0, 
													(struct sockaddr *) &server_address, sizeof(server_address));
	printf("sent ACK: %d bytes using header\n", bytes_sent);
	print_header(header);
	printf("\n");

//this is here because of the extra loop
   num_bytes_received = recvfrom(sockfd, segment, 1024, 0, (struct sockaddr *) &server_address, &saddr_len);
	printf("#0 received %d bytes...\n", num_bytes_received);
	print_header(header);

//----------------------------------------------------------
//                     OPEN the Test File
//----------------------------------------------------------
/*
	char app_buffer[APP_BUF_MAX + 1];
	int elements_read;
	FILE *fp = fopen("test.txt", "r");
	if (fp != NULL) {
		elements_read = fread(app_buffer, sizeof(char), APP_BUF_MAX, fp);
		if (elements_read == 0) {
			fputs("Error reading file\n", stderr);
			exit (EXIT_FAILURE);
		} else {
		app_buffer[++elements_read] = '\0';
		}
	fclose(fp);
	}

	printf("opened file; size of the file to send: %d\n", strlen(app_buffer));
*/

//----------------------------------------------------------
//                     Get Ready to Send
//----------------------------------------------------------

	char send_buffer[536];
	strcpy(send_buffer, "This is a test!"); //this is the message being sent

	int i = 0;
	int index = 0;
//	int elements_left = elements_read;
	int count = 1;

//	int return_value;
//	if (elements_read > 536) {
//		while(index < elements_read) {
//			printf("\nelements left to send: %d\n", elements_left);
//			strncpy(send_buffer, &app_buffer[index], min(elements_left, 536));

//----------------------------------------------------------
//          Initialize Header before Sending Data
//----------------------------------------------------------

//need to send from a real client as this is made up (somewhat)
			header->dest_port 	= header->source_port;
			header->source_port 	= 2222;
			header->seq_num 		= 1001;
			header->ack_num 		= 101;
			header->data_offset 	= 5; 
			header->reserved 		= 0;
			header->ns_flag 		= 0;
			header->cwr_flag 		= 0;
			header->ece_flag 		= 0;
			header->urg_flag 		= 0;
			header->ack_flag 		= 1;
			header->psh_flag 		= 0;
			header->rst_flag 		= 0;
			header->syn_flag 		= 0;
			header->fin_flag 		= 0;
			header->window_size 	= 4096;
			header->checksum 		= 0;
			header->urg_ptr 		= 0;

			printf("data send %d\n", count);
			printf("sending %d bytes with header:\n", strlen(send_buffer));
			printf("send buffer:\n%s", send_buffer);
			printf("\n\n");

//			printf("string length of send buffer: %d\n", strlen(send_buffer));

			int return_value = send_207(3, send_buffer, strlen(send_buffer) + 1, 0); 
			printf("sent %d bytes...\n", return_value);
			count++;

//			header->seq_num = header->seq_num + return_value;


   num_bytes_received = recvfrom(sockfd, segment, 1024, 0, (struct sockaddr *) &server_address, &saddr_len);
	printf("#1 received %d bytes...\n", num_bytes_received);
	print_header(header);

   num_bytes_received = recvfrom(sockfd, segment, 1024, 0, (struct sockaddr *) &server_address, &saddr_len);
	printf("#2 received %d bytes...\n", num_bytes_received);
	print_header(header);


	close (sockfd);
	return 0;
}

//----------------------------------------------------------
//               Function Definitions
//----------------------------------------------------------

void print_header (struct tcp_header *header)
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

int send_207(int sockfd, const unsigned char *buffer, uint32_t buffer_size, int flags)
{	
	int i;
	int index;
	for (i = 0; i < 10; i++){
		if (server_tcb[i].sd == sockfd)
			index = i;
	}

	int socket = server_tcb[index].sd;
	char *payload;
	payload = &segment[sizeof (struct tcp_header)];
	strcpy(payload, buffer); //use memcpy 

	print_header(header);
	printf("\n");

	struct sockaddr_in dest = server_tcb[index].dest_addr;
	socklen_t dest_len = sizeof(dest);
	int rv = sendto(sockfd, segment, 20 + strlen(payload) + 1, flags, (struct sockaddr *) &dest, sizeof(dest));

	zero_header(header);

   int num_bytes_received = recvfrom(sockfd, segment, 1024, 0, (struct sockaddr *) &dest, &dest_len);
	printf("\nreceived %d bytes (should be an ack)...\n", num_bytes_received);
	print_header(header);

	return rv;
}

	struct sockaddr_in server_address;
	socklen_t saddr_len = sizeof(server_address);


void zero_header (struct tcp_header *header)
{
	header->source_port 	= 0;
	header->dest_port 	= 0;
	header->seq_num 		= 0;
	header->ack_num 		= 0;
	header->data_offset 	= 0; 
	header->reserved 		= 0;
	header->ns_flag 		= 0;
	header->cwr_flag 		= 0;
	header->ece_flag 		= 0;
	header->urg_flag 		= 0;
	header->ack_flag 		= 0;
	header->psh_flag 		= 0;
	header->rst_flag 		= 0;
	header->syn_flag 		= 0;
	header->fin_flag 		= 0;
	header->window_size 	= 0;
	header->checksum 		= 0;
	header->urg_ptr 		= 0;
	return;
}






