#include "207layer.h"


int cmpe207_socket(int ai_family, int ai_socktype, int ai_protocol)
{
	
	int check;
	int sockfd_207 = -1;
	if(ai_family != CMPE207_FAM)
		die("unsupported family");
	else if(ai_socktype != CMPE207_SOC)
		die("unsupported socket type");
	else if (ai_protocol != CMPE207_PROC)
		die("unsupported protocol");

	for(check = 0; check < MAX_SOCKET; check++)
	{
		if(CB[check].sock_in_use ==0) //if found one that is open
		{
			sockfd_207 = check;
			CB[sockfd_207].sock_in_use = 1;
			
			struct sockaddr_in *UDP = malloc (sizeof *UDP);
			CB[sockfd_207].sock_struct_UDP = UDP;
					
//create UDP socket	
			CB[sockfd_207].sockfd_udp = socket(PF_INET,SOCK_DGRAM, 0);
			check_for_error(CB[sockfd_207].sockfd_udp,"no UDP socket available.\n");		
			break;		
		}
	}
	//check_for_error(sockfd_207, "no socket available.");

	return sockfd_207;
}
