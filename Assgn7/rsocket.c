#include "rsocket.h"

void HandleRetransmit()
{
	if (uack_count)
	{
		int i;
		for (i = 0; i < uack_count; i++)
		{
			msg here = unack_msg_table[i];
			time_t now = time(NULL);
			if (now - here.time > TIME_THRESH)
			{
				unack_msg_table[i].time = now;
				sendto(sockfd, here.buf, here.len, 0, here.dest_addr, here.addrlen);
			}
		}
	}
}

void HandleAppMsgRecv(int header, char* buf, char* content, int n)
{
	int i;
	int found = 0;
	for (i = 0; i < recv_count; i++)
	{
		if (i == header)
		{
			found = 1;
			break;
		}
	}
	if (found == 0)
	{
		memcpy(content, buf + sizeof(int), n - sizeof(int));
		recv_buffer = (char *) realloc (recv_buffer_size + n - sizeof(int));
		memcpy(recv_buffer + recv_buffer_size, content, n - sizeof(int));
		recv_buffer_size += n - sizeof(int);
		recv_msg_table[recv_count++] = header;
	}
	sendAck(header);
}

void HandleReceive()
{
	struct sockaddr_in cliaddr;
	int clilen = sizeof(cliaddr);
	char buf[100], content[100 - sizeof(int)];
	int n = recvfrom(sockfd, buf, 100, ( struct sockaddr *) &cliaddr, clilen);
	int header;
	memcpy(&header, buf, sizeof(int));
	if (header == 1234) // Ack
	{
		int counter_here, found = 0;
		memcpy(&counter_here, buf + sizeof(int), sizeof(int));
		int i;
		for (i = 0; i < uack_count; i++)
		{
			int counter_only_here;
			memcpy(&counter_only_here, unack_msg_table[i].buf, sizeof(int));
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
	else
	{
		HandleAppMsgRecv(header, buf, content, n);
		recv_msg_table[recv_count].buf = buf+sizeof(int);
		recv_msg_table[recv_count++].addr = cliaddr;
	}
}

void* threadX(void *vargp) 
{ 
	fd_set sock;
	struct timeval tv;
	tv.tv_sec = 2;
	while(1)
	{
		FD_ZERO(&sock);
		FD_SET(sockfd, &sock);

		int selected = select(sockfd + 1, &sock, NULL, NULL, tv);
		if (FD_ISSET(sockfd, &sock))
			HandleReceive();
		else
			HandleRetransmit();

	}
} 

int r_socket()
{
	sockfd = socket(AF_INET, SOCK_DGRAM, 0);
	if (sockfd < 0)
	{
		perror("Unable to create socket");
		return sockfd;
	}
	pthread_create(&X, NULL, threadX, NULL);
	unack_msg_table = (msg *) malloc(100 * sizeof(msg));
	recv_msg_table = (int *) malloc(100 * sizeof(int));
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
			memcpy(buf, &send_count, sizeof(int));
			memcpy(buf + sizeof(int), buf_here + (counter * buf_size), buf_size);
			msg[count].time = time(NULL);
			memcpy(msg[count].buf, &send_count, sizeof(int));
			memcpy(msg[count].buf + sizeof(int), buf_here + (counter * buf_here), buf_size);
			msg[count].len = buf_size;
			uack_count++;
			send_count++;
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
			uack_count++;
			stat = sendto(sockfd, buf, len, flags, dest_addr, addrlen);
		}
		if (stat < 0)
			return stat;
		counter+=1
	}
	return 1;
}

int r_recvfrom(int sockfd, char *buf, int len)
{
	while (recv_count == 0)
		sleep(1);
	int len_to_ret;
	if (recv_msg_table[0].len <= len)
	{
		len_to_ret = recv_msg_table[0].len;
		memcpy(buf, recv_msg_table[0].buf, recv_msg_table[0].len);
	}
	else
	{
		len_to_ret = len;
		memcpy(buf, recv_msg_table[0].buf, len);
	}
	int j;
	for (j = 0; j < recv_count - 1; j++)
		recv_msg_table[j] = recv_msg_table[j + 1];
	recv_count--;
	return len_to_ret;
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