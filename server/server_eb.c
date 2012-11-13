#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <pthread.h>


#include <ifaddrs.h>


#define PORT 9999


//----------------------------------------------------------
//                     TCP Header
//----------------------------------------------------------

//typedef union {
//    char full_20_byte[20]; 
    // assuming little endian other byte order is swapped
    struct tcp_header {
        unsigned short int source_port;
        unsigned short int dest_port;
        unsigned int seq_num;
        unsigned int ack_num;
        unsigned int data_offset : 4; //eb: bits is 4; value is 5
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
//};

//----------------------------------------------------------
//             Segment & Header Position
//----------------------------------------------------------
char segment[556];  //536 byte payload + 20 byte header
struct tcp_header *header = (struct tcp_header *) segment;

//----------------------------------------------------------
//                TCP Control Block
//----------------------------------------------------------

typedef struct Queue  //is part of the control block
{
        int capacity;
        int size;			//current size
        int front;      //first in; next to be removed
        int end;        //end; where we add the next element
        char *elements;
}Queue;

struct tcp_control_block {
	int sd;
	struct sockaddr_in remote_addr;
	struct sockaddr_in local_addr;
	int snd_una; 	//earliest sequence number sent, but not acknowledged
	int snd_nxt; 	//next sequence number to be sent
	int snd_wnd;  	//size of the send window (?)
	int rcv_nxt;   //next sequence number to be received
	int rcv_wnd;   //size of the receive window (advertised by remote host)
	Queue *tcp_send_buffer;  //pointer to the tcp send buffer
	Queue *tcp_recv_buffer;  //pointer to the tcp send buffer
};

struct tcp_control_block server_tcb[10] = {{0}};

//----------------------------------------------------------
//                 Application Buffers
//----------------------------------------------------------

char recv_buffer[4096];  //part of the application...
char send_buffer[4096];

//----------------------------------------------------------
//          Function Declarations - (to header file)
//----------------------------------------------------------

void get_self_ip (char* addressBuffer);
void* rw (void * data);
void print_header (struct tcp_header *header);
int recv_207(int sockfd, char *buffer, ssize_t buffer_size, int flags);
int send_207(int sockfd, const unsigned char *buffer, uint32_t buffer_size, int flags);
Queue * create_queue(int maxElements);
void enqueue(Queue *q_ptr,char element);
void dequeue(Queue *q_ptr);
int front(Queue *q_ptr);

void die (char *s){
	perror(s);
	exit(1);
}

void check_for_error(int ret, char* s){
	if (ret < 0){
		die (s);
	}
}

//----------------------------------------------------------
//                  Program Main
//----------------------------------------------------------

int main (void){

//----------------------------------------------------------
//              Initialize the Header
//----------------------------------------------------------
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

//----------------------------------------------------------
//              Initialize the TCB
// (has to be done somewhere in the handshake process
//----------------------------------------------------------

	server_tcb[0].rcv_nxt = 1;
	server_tcb[0].rcv_wnd = 4096;
	server_tcb[0].snd_una = 1;
	server_tcb[0].sd = 3;
	server_tcb[0].tcp_send_buffer = create_queue(4096);
	server_tcb[0].tcp_recv_buffer = create_queue(4096);


	int sockfd;
	sockfd = socket (AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	if (sockfd < 0) {
		die ("socket()");
	}

	struct sockaddr_in s_server;
	memset((char *) &s_server, 0, sizeof(s_server));
	s_server.sin_family = AF_INET;
  	s_server.sin_port = htons(PORT);
  	s_server.sin_addr.s_addr = htonl(INADDR_ANY);	
  	if (bind (sockfd, (struct sockaddr *) &s_server, sizeof(s_server)) < 0){
  		die ("bind()");
	}

	char self_addr[INET_ADDRSTRLEN];
	get_self_ip (self_addr);
	printf ("== %s : %i ==\n", self_addr, PORT);
	printf ("...waiting for clients...\n");

  //run forever
	for(;;){

	int rv01 = recv_207(3, recv_buffer, sizeof(recv_buffer), 0); // sockfd (3) will not be hard-coded
	printf("server received %d bytes\n", rv01);
	printf("server received payload: %s\n", recv_buffer);

// for testing, echo back the client's message....
	char *tmp_buffer;
	strcpy(tmp_buffer, recv_buffer);
	int rv02 = send_207(3, tmp_buffer, sizeof(tmp_buffer), 0);
// end testing here

  	}
  	close (sockfd);
  	return 0;
}

void get_self_ip (char* addressBuffer){
	struct ifaddrs * ifAddrStruct = NULL;
	struct ifaddrs * ifa = NULL;
   void * tmpAddrPtr = NULL;
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



//----------------------------------------------------------
//               Function Definitions - added
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
	int index;
	for (int i = 0; i < 10; i++){
		if (server_tcb[i].sd == sockfd)
			index = i;
	}

	char *payload;	
	int bytes_read = 0;
	int bytes_to_read = strlen(buffer);
	printf("bytes to read into tcp_send_buffer: %d\n", bytes_to_read);
	int bytes_sent = 0;
	
	while (bytes_read != bytes_to_read) { //data sent by app may be more than segment size; loop

		// if unack'd in sending buffer > rcv.wnd; block, don't send anything
		while (server_tcb[index].tcp_send_buffer->size >= server_tcb[0].rcv_wnd){ 

			sleep(1); // probably signal or something as discussed in class; not sleep
		}

		header->seq_num = server_tcb[index].snd_nxt;                 	//set the header sequence number
      server_tcb[index].snd_nxt += sizeof(segment);						//update snd.nxt
		header->ack_num = server_tcb[index].rcv_nxt;							//set the header ack number

		server_tcb[index].snd_wnd = server_tcb[index].tcp_recv_buffer->size //calculate window size
											 - (server_tcb[index].snd_nxt
											 +	 server_tcb[index].snd_una);

		header->window_size = server_tcb[index].snd_wnd;               //set the header window size

	   payload = &segment[sizeof (struct tcp_header)];          		//position payload in segment
		for (int i = bytes_read; i < 536 + bytes_read; i++) {
				enqueue(server_tcb[index].tcp_send_buffer, buffer[i]); 	//buffer the payload until receive ack
				payload[i -bytes_read] = buffer [i]; 	             		//construct payload for send	
		}
		bytes_read += strlen(payload);            //NOT SURE IF THIS WORKS IF MORE THAN ONE SEGMENT NEEDED
																//OR THE WHOLE LOOP LOGIC ABOVE FOR THAT MATTER
		
		printf("bytes read into tcp_send_buffer: %d\n", bytes_read);		
		printf("payload about to be echoed by server: %s\n", payload);

		int socket = server_tcb[index].sd;
		struct sockaddr_in remote = server_tcb[index].remote_addr;
		int rv = sendto(sockfd, segment, sizeof(segment), flags, (struct sockaddr*)&remote, sizeof(remote));
      //add some error checking...

		bytes_sent += rv;  //keep a running total of bytes sent
		printf("bytes sent by server: %d\n", rv);		
	} // end first while
	return bytes_sent;
}


int recv_207(int sockfd, char *buffer, ssize_t buffer_size, int flags)
{	    
	struct sockaddr_in address;
	socklen_t addr_size = sizeof(address);
  	int rv = recvfrom (sockfd, segment, sizeof(segment), 0, (struct sockaddr *) &address, &addr_size);
      //add some error checking...

	struct tcp_header *header = (struct tcp_header *) segment;		
	if (header->ack_flag != 1)
		; // illegal segment, do we send a reset and exit here?

	int index;
	for (int i = 0; i < 10; i++){  						  //find the index of this socket in the tcb
		if (sockfd == server_tcb[i].sd) {
			index = i;
		}
	}
  	
	server_tcb[index].remote_addr = address; 	       //save the client address (should be done earlier)	
	server_tcb[index].rcv_wnd = header->window_size; //set send window equal to advertised window size

	if (header->ack_num > server_tcb[index].snd_una) {                    //if ack is > last acknowledgment
      int bytes_to_remove = header->ack_num - server_tcb[index].snd_una; //amount of ack'd data
		for (int i = 1; i < server_tcb[index].snd_una; i++)                //take ack'd data out of send buffer
			dequeue(server_tcb[index].tcp_send_buffer);
		server_tcb[index].snd_una = header->ack_num + 1; 	//update send.una if an ack received
	}

	char *payload;                    
	payload = &segment[sizeof (struct tcp_header)];			//extract the payload	

	if(header->seq_num == server_tcb[index].rcv_nxt) {		// if true, segment been received in order
		strcat(buffer, payload);
		server_tcb[index].rcv_nxt = server_tcb[index].rcv_nxt + rv; //update next expected sequence number
		}
		else
		; //temporary buffer if out of order (?)
		  //i think this is going to have to store the entire segment and then go through the above
		  //check again; i'm not sure how to do that; assume in order for now.
	return rv;
}

Queue * create_queue(int maxElements)
{
        Queue *q_ptr;
        q_ptr = (Queue *)malloc(sizeof(Queue));
        q_ptr->elements = (char *)malloc(sizeof(char)*maxElements);
        q_ptr->size 		= 0;
        q_ptr->capacity = maxElements;
        q_ptr->front 	= 0;
        q_ptr->end 		= -1;
        return q_ptr;
}

void enqueue(Queue *q_ptr,char element)
{
	if(q_ptr->size == q_ptr->capacity)
		printf("queue is full; drop segment(?)\n");
	else {
		q_ptr->size++;
		q_ptr->end = q_ptr->end + 1;
		if(q_ptr->end == q_ptr->capacity)  	 	//wrap around if at end of array
			q_ptr->end = 0;
		q_ptr->elements[q_ptr->end] = element; //store the element (char)
	}
return;
}


void dequeue(Queue *q_ptr)
{
	if(q_ptr->size==0) {
		printf("queue is emptyp; noting to dequeue\n");
		return;
	}
   else {
		q_ptr->size--;     						//size is one smaller
		q_ptr->front++;    						//front (next one removed) moves over 1
		if(q_ptr->front==q_ptr->capacity) 	//wrap around if necessary
			q_ptr->front=0;
      }
return;
}

int front(Queue *q_ptr)
{
	if(q_ptr->size==0) {
		printf("Queue is Empty\n");
		exit(0);
	}
return q_ptr->elements[q_ptr->front];
}

