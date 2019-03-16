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
	pthread_create(&X, NULL, threadX, NULL);
	unack_msg_table = (msg *) malloc(100 * sizeof(msg));
	recv_msg_table = (msg *) malloc(100 * sizeof(msg));
	buf = (char *) malloc(100 * sizeof(char));
	usleep(10000);
	count++;
	return sockfd;
}

int r_sendto(int sockfd, const void *buf_here, size_t len, int flags, const struct sockaddr *dest_addr, socklen_t addrlen)
{
	int counter = 0;
	int buf_size = 100 - sizeof(int);
	while(1)
	{
		int stat;
		if (len > buf_size)
		{
			memcpy(buf, &counter, sizeof(int));
			memcpy(buf + sizeof(int), buf_here + (count * buf_size), buf_size);
			msg[count].time = time(NULL);
			memcpy(msg[count].buf, &counter, sizeof(int));
			memcpy(msg[count].buf + sizeof(int), buf_here + (count * buf_here), buf_size);
			msg[count].len = buf_size;
			stat = sendto(sockfd, buf, buf_size, flags, dest_addr, addrlen);
			len -= buf_size;
		}
		else
		{
			memcpy(buf, &counter, sizeof(int));
			memcpy(buf + sizeof(int), buf_here + (count * buf_size), len);
			msg[count].time = time(NULL);
			memcpy(msg[count].buf, &counter, sizeof(int));
			memcpy(msg[count].buf + sizeof(int), buf_here + (count * buf_here), len);
			msg[count].len = len;
			stat = sendto(sockfd, buf, len, flags, dest_addr, addrlen);
		}
		if (stat < 0)
			return stat;
		counter+=1
	}
	return 1;
}