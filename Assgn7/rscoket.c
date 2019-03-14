#include "rsocket.h"

void* threadX(void *vargp) 
{ 
	int count_here = count;
    sleep(1); 
    printf("Printing GeeksQuiz from Thread \n"); 
    return NULL; 
} 

int r_socket()
{
	int sockfd = socket(AF_INET, SOCK_DGRAM, 0);
	if (sockfd < 0)
	{
		perror("Unable to create socket");
		return sockfd;
	}
	pthread_create(&X[count], NULL, threadX, NULL);
	unack_msg_table[count] = (int *) malloc(100 * sizeof(int));
	recv_msg_table[count] = (int *) malloc(100 * sizeof(int));
	buf[count] = (char *) malloc(100 * sizeof(char));
	mapping[count] = sockfd;
	usleep(10000);
	count++;
	return sockfd;
}

int r_sendto(int sockfd, const void *buf, size_t len, int flags, const struct sockaddr *dest_addr, socklen_t addrlen)
{
	int counter = 0;
	if (len > (100 - sizeof(int)))
	{

	}
}