#include "207layer.h"


int cmpe207_bind(int sockfd, struct sockaddr_in *addr, socklen_t addrlen)
{
	int ret=-1;
	
	if (gTcp_Block[sockfd].sock_in_use == 0)
		die("bind(): socket not in use.");
	else if (gTcp_Block[sockfd].pSocket_info->sin_addr.s_addr != 0)
		die("bind(): socket already has IP.");

	if (sockfd == 0) //first socket
	{
		gTcp_Block[sockfd].pSocket_info->sin_port = htons(UDP_PORT);
		gTcp_Block[sockfd].cmpe207_port = CMPE207_PORT_BASE;
	}
//fill 207 port
	else
	{
		int port = -1;
		int check;
		for(check = 1; check < MAX_PORT; check++)
		{
			if(cmpe207_port_in_use[check]==0) //if found one that is open
			{
				port = check + CMPE207_PORT_BASE;
				cmpe207_port_in_use[check] = 1;
				gTcp_Block[sockfd].cmpe207_port = port; 
				break;		
			}
		}

		check_for_error(port,"cmpe207 bind fail: change MAX_PORT in 207layer.h.");
	}

//copy addr and family to udp struct
	gTcp_Block[sockfd].pSocket_info->sin_addr = addr->sin_addr;
	gTcp_Block[sockfd].pSocket_info->sin_family = addr->sin_family;

//UDP bind
	ret = bind(gTcp_Block[sockfd].sockfd_udp,(struct sockaddr *) (gTcp_Block[sockfd].pSocket_info), addrlen);

	check_for_error(ret,"udp bind fail.");
	
	socklen_t size = sizeof(gTcp_Block[sockfd].pSocket_info);
	check_for_error(getsockname(gTcp_Block[sockfd].sockfd_udp, (struct sockaddr *) gTcp_Block[sockfd].pSocket_info, &size),"getsockname"); 

	printf("UDP bind(IP:sockfd_UDP:UDP_port): %s:%d:%d\n", inet_ntoa(gTcp_Block[sockfd].pSocket_info->sin_addr), gTcp_Block[sockfd].sockfd_udp, htons(gTcp_Block[sockfd].pSocket_info->sin_port));

	printf("207 bind(IP:sockfd_207:207_port): %s:%d:%d\n\n", inet_ntoa(gTcp_Block[sockfd].pSocket_info->sin_addr), sockfd, gTcp_Block[sockfd].cmpe207_port);
	
	return ret;
}

