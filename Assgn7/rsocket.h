#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define PORT 5000

struct msg
{
	time_t time;
	char buf[100];
	int len;
	struct sockaddr* dest_addr;
	socklen_t addrlen;
};

struct recv_msg
{
	char* buf;
	int len;
	struct sockaddr* addr;

};

typedef struct msg msg;
typedef struct recv_msg recv_msg;

msg* uack_msg_table;
recv_msg* recv_msg_table;
char *recv_buffer;	 // need to store port
int recv_buffer_size = 0;
int recv_size;
int uack_count = 0;
int recv_count = 0;
int sockfd;
char* buf;
int count = 0;
int send_count = 0;
pthread_t X; 

struct socket
{
	int sockfd;
	int dest_port;
};

typedef struct socket socket;

int r_socket(socket*);

int r_sendto(int, char*, int, x, const struct  sockaddr *, int)
{
	
};