/****************
Assignment 7
Sayan Sinha
16CS10048
****************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <pthread.h>

#define SOCK_MRP SOCK_DGRAM

int r_socket(int, int, int);

int r_sendto(int, const void*, size_t, int, const struct sockaddr*, socklen_t);

int r_recvfrom(int, char*, size_t, int, const struct sockaddr *, socklen_t);

int r_bind(int, const struct sockaddr*, socklen_t);

int r_close(int);