#include <stdio.h>
#include <stdlib.h>
#include <string.h>

enum FLAGS {CLOSED, LISTEN, SYN_RCVD, SYN_SENT, ESTABLISHED, FIN_WAIT_1, CLOSE_WAIT, FIN_WAIT_2, CLOSING, LAST_ACK, TIME_WAIT};

#define URG (1<<5)
#define ACK (1<<4)
#define PSH (1<<3)
#define RST (1<<2)
#define SYN (1<<1)
#define FIN (1<<0)

/* for checking the bits
	for(i=0; i<6;i++)
	{
		printf("%i",(flag>>i)&1);
	}		
	printf("\t");
*/
int TCPStateMachine(int flag, int state)
{
	int error = 0;
int i = 0;
	switch (state){
		case CLOSED:
			break;
		case LISTEN:
			if(flag == SYN){
				state = SYN_RCVD;
				printf("--SYN_RCVD--\n");
				//send syn+ack
			}
			//send -> SYN_SENT
			//send syn	
			//close -> CLOSED
			else
				error = 1;
			break;
		case SYN_RCVD:
			if(flag == ACK){
				state = ESTABLISHED;
				printf("--ESTABLISHED--\n");
			}
			//close
			//send fin
			else
				error = 1;
			break;
		case SYN_SENT:
			if(flag == (SYN||ACK)){
				state = ESTABLISHED;
				printf("--ESTABLISHED--\n");
				//send ack
			}						
			else if(flag == SYN){
				state = SYN_RCVD;
				printf("--SYN_RCVD--\n");
				//send syn+ack
			}	
			//close -> CLOSED 
			else
				error = 1;
			break;
		case ESTABLISHED:
			if(flag == FIN){
				state = CLOSE_WAIT;
				printf("--CLOSE_WAIT--\n");
				//send ack
			}
			//close -> FIN_WAIT_1
			//send fin
			else
				error = 1;
			break;
		case FIN_WAIT_1:
			if(flag == ACK){
				state = FIN_WAIT_2;
				printf("--FIN_WAIT_2--\n");
			}
			else if(flag == FIN){
				state = CLOSING;
				printf("--CLOSING--\n");
				//send ack
			}
			else
				error = 1;
			break;
		case CLOSE_WAIT:
			//close -> LAST_ACK
			//send FIN
			/*else
				error = 1;*/
			break;
		case FIN_WAIT_2:
			if(flag == FIN){
				state = TIME_WAIT;
				printf("--TIME_WAIT--\n");
				//send ack
			}
			else
				error = 1;
			break;
		case CLOSING:
			if(flag == ACK){
				state = TIME_WAIT;
				printf("--TIME_WAIT--\n");
			}
			else
				error = 1;
			break;		
		case LAST_ACK:
			if(flag == ACK){
				state = CLOSED;
				printf("--LAST_ACK--\n");
			}
			else
				error = 1;
			break;
		case TIME_WAIT:
			//start timer 2RTT
			/*else
				error = 1;*/
			break;
		/*case CLOSED:
			break; */
	}
	if(error){ //reset connection
		//close connection
		// send reset
		printf("--RESET--\n");
		error = 0;
		return -1;
	}
	return state;
}/*
int main(void)
{
	char*flagchar[] = {"fin", "syn", "rst", "push", "ack", "urg"};
	int in = 0, count = 0;
	int flag = 0;
	int state = 1; //start off in listen stage

	printf("--LISTEN--\n");
	while(in != 0 | in != 1)
	{
		printf("%s: ",flagchar[count]);
		scanf("%d",&in);

		flag |=(in<<count);

		count = (++count)%6;
		if(count==0)
		{	
			if((state = TCPStateMachine(flag,state)) == -1)		
				break; 			
			flag =0;
			printf("\n");
		}
	}
	return 0;
}*/
