#include "rsocket.h"

void HandleRetransmit(int sockfd)
{
	// printf("Will retransmit\n");
	if (uack_count)
	{
		// printf("Yeah Will retransmit\n");
		int i;
		for (i = 0; i < uack_count; i++)
		{
			msg here = unack_msg_table[i];
			time_t now = time(NULL);
			// printf("Time gone: %d\n", (int)(now - here.time));
			// printf("Now time: %d\n", (int)(now));
			// printf("Here time: %d\n", (int)(here.time));
			if ((int)(now - here.time) >= TIME_THRESH)
			{
				unack_msg_table[i].time = now;
				char buf_total_here[sizeof(short) + 100];
				memcpy(buf_total_here, &(here.counter), sizeof(short));
				memcpy(buf_total_here + sizeof(short), here.buf, here.len);
				sendto(sockfd, buf_total_here, sizeof(short) + here.len, 0, here.dest_addr, here.addrlen);
				printf("Retransmitted\n");
				// exit(0);
			}
		}
	}
}

void sendAck(int sockfd, short header, int i)
{
	char buf_temp[2* sizeof(short)];
	short prepend = 1234;
	memcpy(buf_temp, &prepend, sizeof(short));
	memcpy(buf_temp + sizeof(short), &header, sizeof(short));
	sendto(sockfd, buf_temp, 2 * sizeof(short), 0, (struct sockaddr *) &recv_msg_table[i].addr, sizeof(recv_msg_table[i].addr));
	printf("Ack sent\n");
}

void HandleAppMsgRecv(int sockfd, short header, char* buf, int n, struct sockaddr_in addr)
{
	printf("In app receive\n");
	int i;
	int found = 0;
	printf("Recv count: %d\n", recv_count);
	for (i = 0; i < recv_count; i++)
	{
		if (recv_msg_table[i].counter == header)
		{
			found = 1;
			break;
		}
	}
	if (found == 0)
	{
		memcpy(recv_buffer[recv_buffer_count].buf, buf + sizeof(short), n - sizeof(short));
		recv_buffer[recv_buffer_count].len = n - sizeof(short);
		recv_buffer[recv_buffer_count].addr = addr;
		recv_buffer_count++;
		printf("Before recv_msg_table update\n", n);
		recv_msg_table[recv_count].counter = header;
		printf("After recv_msg_table update\n");
		recv_msg_table[recv_count].addr = addr;
		recv_count++;
	}
	sendAck(sockfd, header, i);
}

void HandleACKMsgRecv(char* buf)
{
	printf("In ack receive\n");
	short counter_here, found = 0;
	memcpy(&counter_here, buf + sizeof(short), sizeof(short));
	int i;
	for (i = 0; i < uack_count; i++)
	{
		short counter_only_here = unack_msg_table[i].counter;
		if (counter_only_here == counter_here)
		{
			found = 1;
			break;
		}
	}
	if (found)
	{
		int j;
		for (j = i; j < uack_count - 1; j++)
			unack_msg_table[j] = unack_msg_table[j + 1];
		uack_count--;
	}
}

void HandleReceive(int sockfd)
{
	struct sockaddr_in cliaddr;
	memset(&cliaddr, 0, sizeof(cliaddr));
	int clilen = sizeof(cliaddr);
	char bufn[sizeof(short) + 100];
	int n = recvfrom(sockfd, bufn, sizeof(short) + 100, 0, ( struct sockaddr *) &cliaddr, &clilen);
	if (n < 0)
	{
		perror("Error in receipt");
		exit(EXIT_FAILURE);
	}
	printf("Received internal: %s\nn is %d\n", bufn+sizeof(short), n);
	short header;
	memcpy(&header, bufn, sizeof(short));
	if (header > 100) // Ack
		HandleACKMsgRecv(bufn);	
	else
		HandleAppMsgRecv(sockfd, header, bufn, n, cliaddr);
}

void* threadX(void* vargp) 
{
	int sockfd = *((int *) vargp); 
	fd_set sock;
	struct timeval tv;
	tv.tv_sec = 2;
	while(1)
	{
		FD_ZERO(&sock);
		FD_SET(sockfd, &sock);

		int selected = select(sockfd + 1, &sock, NULL, NULL, &tv);
		// printf("Timeout\n");
		if (FD_ISSET(sockfd, &sock))
		{
			printf("Recv\n");
			HandleReceive(sockfd);
		}
		else
		{
			// printf("Retransmit\n");
			HandleRetransmit(sockfd);
		}

	}
} 

int r_socket(int domain, int type, int protocol)
{
	int sockfd = socket(domain, SOCK_DGRAM, protocol);
	if (sockfd < 0)
	{
		perror("Unable to create socket");
		return sockfd;
	}
	int *arg = malloc(sizeof(*arg));
	*arg = sockfd;
	pthread_create(&X, NULL, threadX, arg);
	unack_msg_table = (msg *) malloc(100 * sizeof(msg));
	recv_msg_table = (recv_msg *) malloc(100 * sizeof(recv_msg));
	recv_buffer = (recv_buf *) malloc(100 * sizeof(recv_buffer));
	buf_total = (char *) malloc(sizeof(short) + (100 * sizeof(char)));
	recv_buffer_count = 0;
	recv_size = 0;
	uack_count = 0;
	recv_count = 0;
	send_count = 0;
	usleep(10000);
	// count++;
	return sockfd;
}

int r_sendto(int sockfd, const void* buf_here, size_t len, const struct sockaddr* dest_addr, socklen_t addrlen)
{
	int counter = 0;
	int buf_size = 100;
	while(1)
	{
		printf("len: %d\n", (int)len<=0);
		if ((int)len <= 0)
		{
			printf("Break please\n");
			break;
		}
		int stat, amt;
		if (len > buf_size)
			amt = buf_size;
		else
			amt = len;
		printf("Here1\n");
		memcpy(buf_total, &send_count, sizeof(short));
		printf("Here2\n");
		memcpy(buf_total + sizeof(short), (char *) (buf_here + (counter * buf_size)), amt);
		printf("Here3\n");
		unack_msg_table[uack_count].time = time(NULL);
		unack_msg_table[uack_count].counter = send_count;
		memcpy(unack_msg_table[uack_count].buf, buf_here + (counter * buf_size), amt);
		unack_msg_table[uack_count].len = amt;
		unack_msg_table[uack_count].dest_addr = dest_addr;
		unack_msg_table[uack_count].addrlen = addrlen;
		uack_count++;
		send_count++;
		stat = sendto(sockfd, buf_total, sizeof(short) + amt, 0, dest_addr, addrlen);
		len -= buf_size;
		
		printf("stat: %d\n", stat);
		if (stat < 0)
			return stat;
		counter+=1;
	}
	return 1;
}

int r_recvfrom(int sockfd, char *buf, int len, const struct  sockaddr * addr, socklen_t addrlen)
{
	while (recv_buffer_count == 0)
		sleep(1);
	int len_to_ret;
	if (recv_buffer[0].len <= len)
		len_to_ret = recv_buffer[0].len;
	else
		len_to_ret = len;
	memcpy(buf, recv_buffer[0].buf, len_to_ret);
	memcpy(addr, (struct sockaddr *) &recv_buffer[0].addr, sizeof(recv_buffer[0].addr));
	int j;
	for (j = 0; j < recv_buffer_count - 1; j++)
		recv_buffer[j] = recv_buffer[j + 1];
	recv_buffer_count--;
	return len_to_ret;
}

int r_bind(int sockfd, const struct sockaddr* servaddr,  socklen_t addrlen)
{
	return bind(sockfd, servaddr,  addrlen);
}

int r_close(int sockfd)
{
	while(uack_count);
	free(unack_msg_table);
	free(recv_msg_table);
	free(recv_buffer);
	pthread_kill(&X);
	close(sockfd);
}