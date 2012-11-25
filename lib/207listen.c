#include "207layer.h"


int cmpe207_listen(int sockfd, int backlog)
{
	char* msg = "server";
	struct packet_header pheader;

	int current_state = gTcp_Block[sockfd].tcp_current_state;

//error checking: check sockfd IN USE, in CLOSED state
	if(!gTcp_Block[sockfd].sock_in_use)
		die("listen(): socket not in use.");
	if(!check_state(sockfd, "CLOSED"))
		die("listen(): not in CLOSED state.");

	gTcp_Block[sockfd].queue_size = backlog;

	gTcp_Block[sockfd].tcp_current_state = get_tcp_state(current_state, pheader, msg);

	current_state = gTcp_Block[sockfd].tcp_current_state;

	printf("listen: current state = %s\n", (char *)get_state_name(current_state));
}


