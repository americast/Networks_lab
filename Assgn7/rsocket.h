#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define PORT 5000

struct msg
{
	time_t time;
	char buf[100];
	int len;
	int dest_port;
};

typedef struct msg msg;

msg* uack_msg_table;
msg* recv_msg_table;
char* buf;
int count = 0;
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