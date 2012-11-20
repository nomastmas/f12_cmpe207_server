#include "207layer.h"

#include <sys/ioctl.h>
#include <net/if.h>

Control_Block CB[MAX_SOCKET] = {0};
int cmpe207_port_in_use [MAX_PORT] = {0};

int main()
{
//struct for bind
	struct sockaddr_in addr = {0};

//test cmpe207_socket
	int check, sockfd_207, ret;
	for(check = 0; check<=MAX_SOCKET; check++)
	{
		sockfd_207 = cmpe207_socket(CMPE207_FAM, CMPE207_SOC, CMPE207_PROC);

		if(sockfd_207 != -1)		
			printf("UDP socket %d created \n", CB[sockfd_207].sockfd_udp);
//remember to take this else out (debug purpose only) and uncomment:
//check_for_error(sockfd_207, "no socket available."); in 207sock.c 
		else{
			printf("no 207 socket available: change MAX_SOCK in 207layer.h.\n\n");
			break;
		}
	}

//find IP for TCP header with SIOGCIFCONF
	struct ifreq *ifr;
	struct ifconf ifc;
	int s = 4;
	int num_interfaces;

	// find number of interfaces.
	memset(&ifc, 0, sizeof(ifc));
	ifc.ifc_ifcu.ifcu_req = NULL;
	ifc.ifc_len = 0;

	check_for_error(ioctl(s, SIOCGIFCONF, &ifc), "ioctl");

	if ((ifr = malloc(ifc.ifc_len)) == NULL) {
		die("malloc");
	}
	ifc.ifc_ifcu.ifcu_req = ifr;

	check_for_error(ioctl(s, SIOCGIFCONF, &ifc), "ioctl");
	
	//close(s);
	char* self_IP;
	check = -1;
	int i = 0;
	num_interfaces = ifc.ifc_len / sizeof(struct ifreq);
	for (i = 0; i < num_interfaces; i++) {
		struct sockaddr_in *sin = (struct sockaddr_in *)&ifr[i].ifr_addr;
		self_IP = inet_ntoa(addr.sin_addr);		
		if(strcmp(self_IP,"0.0.0.0") ||strcmp(self_IP,"127.0.0.1")) 
		{
			struct sockaddr_in *sin = (struct sockaddr_in *)&ifr[i].ifr_addr;
			addr.sin_addr.s_addr = sin->sin_addr.s_addr;
			//printf("%-8s : %s\n", ifr[i].ifr_name, inet_ntoa(addr.sin_addr));
			check = 1;
		}
	}
	free(ifr);

	check_for_error(check, "Only have loopback addresses.");

//test cmpe207_bind

	addr.sin_family = AF_INET;
	for(sockfd_207 = 0; sockfd_207<MAX_SOCKET; sockfd_207++)
	{
		if(CB[sockfd_207].sock_in_use)
			ret = cmpe207_bind(sockfd_207, &addr, sizeof addr);
	}


	return 0;
}
