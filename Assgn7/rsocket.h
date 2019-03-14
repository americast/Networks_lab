#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define PORT 5000

int* uack_msg_table[100];
int* recv_msg_table[100];
char* buf[100];
int count = 0;
pthread_t X[100]; 
int mapping[100];

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