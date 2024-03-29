/****************
Assignment 7
Sayan Sinha
16CS10048
****************/

#include "rsocket.h"


struct msg // Data type for each message
{
	time_t time;
	short counter;
	char buf[100];
	int flags;
	size_t len;
	struct sockaddr* dest_addr;
	socklen_t addrlen;
};

struct recv_buf // Content of each buffer
{
	char buf[100];
	int len;
	struct sockaddr_in addr;
	socklen_t clilen;
};

struct recv_msg // Received message table content
{
	short counter;
	struct sockaddr_in addr;
};

typedef struct msg msg;				
typedef struct recv_msg recv_msg;
typedef struct recv_buf recv_buf;


msg* unack_msg_table;		// Unacknowledged message table
recv_msg* recv_msg_table;	// Received message id table
recv_buf* recv_buffer;		// Reveive buffer
int recv_buffer_count;		// No of items in the receive buffer
int uack_count;				// No of unacknowledged sent items
int recv_count;				// Total no of items received
int sockfd_here;			// Copy of the initialised sockfd for reference
char* buf_total;			// Buffer to be filled before sending
// int count;
short send_count;			// The counter which acts as a header
pthread_t X; 				// The thread X which takes care of receive
pthread_mutex_t lock_uack_count, lock_recv_buffer_count, lock_prob_sent_counter; 
pthread_mutex_t lock_uack_msg_table, lock_recv_buffer; // Mutex lockers
int prob_sent_counter;		// Counts all sending attempts

int dropMessage(float p)	// Message dropping
{
	int num = rand();
	num = (int)num % 1000;
	if (num/1000.0 <= p)
		return 1;
	return 0;
}

void HandleRetransmit(int sockfd)	// Handles retransmission
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
				sendto(sockfd, buf_total_here, sizeof(short) + here.len, here.flags, here.dest_addr, here.addrlen);
				pthread_mutex_lock(&lock_prob_sent_counter);
				prob_sent_counter++;
				pthread_mutex_unlock(&lock_prob_sent_counter);
				printf("Retransmitted %s\n", here.buf);
				// exit(0);
			}
		}
	}
}

void sendAck(int sockfd, short header, int i)	// Sends acknowledgement
{
	char buf_temp[2* sizeof(short)];
	short prepend = 1234;
	memcpy(buf_temp, &prepend, sizeof(short));
	memcpy(buf_temp + sizeof(short), &header, sizeof(short));
	sendto(sockfd, buf_temp, 2 * sizeof(short), 0, (struct sockaddr *) &recv_msg_table[i].addr, sizeof(recv_msg_table[i].addr));
}

void HandleAppMsgRecv(int sockfd, short header, char* buf, int n, struct sockaddr_in addr, socklen_t clen) // Writes to receive buffer on messsage receipt
{
	int i;
	int found = 0;
	for (i = 0; i < recv_count; i++)
	{
		if (recv_msg_table[i].counter == header)
		{
			found = 1;
			recv_msg_table[i].addr = addr;
			break;
		}
	}
	if (found == 0)
	{
		pthread_mutex_lock(&lock_recv_buffer);
		memcpy(recv_buffer[recv_buffer_count].buf, buf + sizeof(short), n - sizeof(short));
		recv_buffer[recv_buffer_count].len = n - sizeof(short);
		recv_buffer[recv_buffer_count].addr = addr;
		recv_buffer[recv_buffer_count].clilen = clen;
		pthread_mutex_unlock(&lock_recv_buffer);
		pthread_mutex_lock(&lock_recv_buffer_count);
		recv_buffer_count++;
		pthread_mutex_unlock(&lock_recv_buffer_count);
		recv_msg_table[recv_count].counter = header;
		recv_msg_table[recv_count].addr = addr;
		recv_count++;
	}
	sendAck(sockfd, header, i);
}

void HandleACKMsgRecv(char* buf)	// Mark sent message as acknoeledged
{
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
		pthread_mutex_lock(&lock_uack_msg_table);
		for (j = i; j < uack_count - 1; j++)
			unack_msg_table[j] = unack_msg_table[j + 1];
		pthread_mutex_unlock(&lock_uack_msg_table);
		pthread_mutex_lock(&lock_uack_count);
		uack_count--;
		pthread_mutex_unlock(&lock_uack_count);
	}
}

void HandleReceive(int sockfd)	// Handle message receipt
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
	// printf("Received internal: %s\nn is %d\n", bufn+sizeof(short), n);
	short header;
	memcpy(&header, bufn, sizeof(short));
	float prob = DROP_PROB;
	if (dropMessage(prob))
		return;
	if (header > 100) // Ack
		HandleACKMsgRecv(bufn);	
	else
		HandleAppMsgRecv(sockfd, header, bufn, n, cliaddr, clilen);
}

void* threadX(void* vargp) 	// Keeps looking for incoming data
{
	int sockfd = *((int *) vargp); 
	fd_set sock;
	struct timeval tv;
	tv.tv_sec = 1;
	while(1)
	{
		FD_ZERO(&sock);
		FD_SET(sockfd, &sock);

		int selected = select(sockfd + 1, &sock, NULL, NULL, &tv);
		// printf("Timeout\n");
		if (FD_ISSET(sockfd, &sock))
			HandleReceive(sockfd);
		else
			HandleRetransmit(sockfd);
	}
} 

int r_socket(int domain, int type, int protocol)	// Creates a socket of type MRP
{
	if (type != SOCK_MRP)
	{
		fprintf(stderr, "Invalid socket type\n");
		return -1;
	}
	srand(time(NULL));
	int sockfd = socket(domain, SOCK_DGRAM, protocol);
	if (sockfd < 0)
		return sockfd;
	int *arg = malloc(sizeof(*arg));
	*arg = sockfd;
	pthread_create(&X, NULL, threadX, arg);
	unack_msg_table = (msg *) malloc(100 * sizeof(msg));
	recv_msg_table = (recv_msg *) malloc(100 * sizeof(recv_msg));
	recv_buffer = (recv_buf *) malloc(100 * sizeof(recv_buffer));
	buf_total = (char *) malloc(sizeof(short) + (100 * sizeof(char)));
	recv_buffer_count = 0;	// Initialise counters to zero
	uack_count = 0;
	recv_count = 0;
	send_count = 0;
	prob_sent_counter = 0;
	usleep(10000);
	// count++;
	sockfd_here = sockfd;
	if(pthread_mutex_init(&lock_uack_count, NULL)!=0)
	{
		perror("Error in mutex init");
		exit(EXIT_FAILURE);
	}
	if(pthread_mutex_init(&lock_recv_buffer, NULL)!=0)
	{
		perror("Error in mutex init");
		exit(EXIT_FAILURE);
	}
	if(pthread_mutex_init(&lock_uack_msg_table, NULL)!=0)
	{
		perror("Error in mutex init");
		exit(EXIT_FAILURE);
	}
	if(pthread_mutex_init(&lock_prob_sent_counter, NULL)!=0)
	{
		perror("Error in mutex init");
		exit(EXIT_FAILURE);
	}
	if(pthread_mutex_init(&lock_recv_buffer_count, NULL)!=0)
	{
		perror("Error in mutex init");
		exit(EXIT_FAILURE);
	}
	return sockfd;
}

int r_sendto(int sockfd, const void* buf_here, size_t len, int flag,const struct sockaddr* dest_addr, socklen_t addrlen)  // Sends data via MRP socket
{
	int counter = 0;
	int buf_size = 100;
	while(1)
	{
		if ((int)len <= 0)
			break;
		int stat, amt;
		if (len > buf_size)
			amt = buf_size;
		else
			amt = len;
		memcpy(buf_total, &send_count, sizeof(short));	// Add header
		memcpy(buf_total + sizeof(short), (char *) (buf_here + (counter * buf_size)), amt); // Add content
		pthread_mutex_lock(&lock_uack_msg_table);
		unack_msg_table[uack_count].time = time(NULL);
		unack_msg_table[uack_count].counter = send_count;
		memcpy(unack_msg_table[uack_count].buf, buf_here + (counter * buf_size), amt);
		unack_msg_table[uack_count].len = amt;
		unack_msg_table[uack_count].flags = flag;
		unack_msg_table[uack_count].dest_addr = dest_addr;
		unack_msg_table[uack_count].addrlen = addrlen;
		pthread_mutex_unlock(&lock_uack_msg_table);
		pthread_mutex_lock(&lock_uack_count);
		uack_count++;
		pthread_mutex_unlock(&lock_uack_count);
		send_count++;
		stat = sendto(sockfd, buf_total, sizeof(short) + amt, flag, dest_addr, addrlen);
		printf("Transmitted %s\n", buf_total + sizeof(short));
		pthread_mutex_lock(&lock_prob_sent_counter);
		prob_sent_counter++;
		pthread_mutex_unlock(&lock_prob_sent_counter);
		len -= buf_size;
		
		if (stat < 0)
		{
			uack_count--;
			send_count--;
			return stat;
		}
		counter+=1;
	}
	return 0;
}

int r_recvfrom(int sockfd, char *buf, size_t len_here, int flag, const struct  sockaddr * addr, socklen_t* addrlen) // Receive data from MRP socket
{
	if (sockfd != sockfd_here)
		return -1;
	int len = (int)len_here;
	if (flag != MSG_PEEK && flag != 0)
		return -1;
	while (recv_buffer_count == 0)
		sleep(1);
	int len_to_ret;
	if (recv_buffer[0].len <= len)
		len_to_ret = recv_buffer[0].len;
	else
		len_to_ret = len;
	memcpy(buf, recv_buffer[0].buf, len_to_ret);	// Retrieve content
	memcpy(addr, (struct sockaddr *) &recv_buffer[0].addr, sizeof(recv_buffer[0].addr)); // Retrieve address
	memcpy(addrlen, &recv_buffer[0].clilen, sizeof(socklen_t));
	int j;
	if (flag != MSG_PEEK)
	{
		pthread_mutex_lock(&lock_recv_buffer);
		for (j = 0; j < recv_buffer_count - 1; j++)
			recv_buffer[j] = recv_buffer[j + 1];
		pthread_mutex_unlock(&lock_recv_buffer);
		pthread_mutex_lock(&lock_recv_buffer_count);
		recv_buffer_count--;
		pthread_mutex_unlock(&lock_recv_buffer_count);
	}
	return len_to_ret;
}

int r_bind(int sockfd, const struct sockaddr* servaddr,  socklen_t addrlen) // binds MRP to an IP and port
{
	return bind(sockfd, servaddr,  addrlen);
}

int r_close(int sockfd)		// Close socket
{
	while(uack_count);
	free(unack_msg_table);	// Free dynamically allocated memory
	free(recv_msg_table);
	free(recv_buffer);
	pthread_cancel(X);		// Close the thread
	close(sockfd);
	pthread_mutex_destroy(&lock_recv_buffer);
	pthread_mutex_destroy(&lock_uack_count);
	pthread_mutex_destroy(&lock_recv_buffer_count);
	pthread_mutex_destroy(&lock_prob_sent_counter);
	pthread_mutex_destroy(&lock_uack_msg_table);
	// char buf_test[] = "A quick brown fox jumps over the lazy dog.";
	// printf("prob_sent_counter %d\n", prob_sent_counter);
	// printf("len of str %d\n", strlen(buf_test));
	printf("\nTotal no of sends: %d\n", prob_sent_counter);
	return 0;
}