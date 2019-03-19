#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <pthread.h>

#define TIME_THRESH 2
#define SOCK_MRP SOCK_DGRAM
#define DROP_PROB 0.1

struct msg
{
	time_t time;
	short counter;
	char buf[100];
	size_t len;
	struct sockaddr* dest_addr;
	socklen_t addrlen;
};

struct recv_buf
{
	char buf[100];
	int len;
	struct sockaddr_in addr;
};

struct recv_msg
{
	short counter;
	struct sockaddr_in addr;
};

typedef struct msg msg;
typedef struct recv_msg recv_msg;
typedef struct recv_buf recv_buf;

msg* unack_msg_table;
recv_msg* recv_msg_table;
recv_buf* recv_buffer;	 // need to store port
int recv_buffer_count;
int recv_size;
int uack_count;
int recv_count;
// int sockfd;
char* buf_total;
// int count;
short send_count;
pthread_t X; 
pthread_mutex_t lock_uack_count, lock_recv_count, lock_recv_buffer_count, lock_prob_sent_counter; 

int prob_sent_counter;

void HandleRetransmit(int);

void sendAck(int, short, int);

void HandleAppMsgRecv(int, short, char*, int, struct sockaddr_in);

void HandleACKMsgRecv(char*);

void HandleReceive(int);

void* threadX(void*);

int r_socket(int, int, int);

int r_sendto(int, const void*, size_t, const struct sockaddr*, socklen_t);

int r_recvfrom(int, char*, int, const struct sockaddr *, socklen_t);

int r_bind(int, const struct sockaddr*, socklen_t);

int r_close(int);

int dropMessage(float);