/**
Non blocking is better because:

1)  There might be race conditions as a signal might be caught while
	some data is being manipulated.
2)	In case the signal handling function is not pseudo-atomic, some 
	receive call may occur while the previous one is being handled.
	This might create loss of receives. It needs to be handled only
	programmatically which might not be scalable.
3)	If some syscall is taking place and a signal needs to be handled,
	the decision taken might be arbitrary and it might not be known 
	if the syscall needs to be restarted, though this might be over-
	come by using appropriate sigaction.
4)	SIGIO signal might be needed by various file descriptors. It is
	insufficient to conclude the signal handler would be called only
	for socket io.

Assignment 9
Sayan Sinha
16CS10048
Client

**/

// The client program
#include <stdio.h> 
#include <stdlib.h> 
#include <unistd.h> 
#include <string.h> 
#include <sys/types.h> 
#include <sys/socket.h> 
#include <arpa/inet.h> 
#include <netinet/in.h> 
#include <fcntl.h>
#include <signal.h>

#define MAXLINE 1024 

int main()
{ 

	int sockfd; 
  
	void my_recv(int signum)
	{
		char buf[100];
		struct sockaddr_in cliaddr;
	    memset(&cliaddr, 0, sizeof(cliaddr)); 
		int len = sizeof(cliaddr);
		int n = recvfrom(sockfd, buf, 100, 0,  (struct sockaddr *) &cliaddr, &len);
		printf("Received: %s\n", buf);
		close(sockfd);
		exit(EXIT_SUCCESS);
	}
    // Creating socket file descriptor 
	struct sockaddr_in servaddr;
    sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if ( sockfd < 0 ) { 
        perror("socket creation failed"); 
        exit(EXIT_FAILURE); 
    } 
  
    memset(&servaddr, 0, sizeof(servaddr)); 
      
    // Server information 
    servaddr.sin_family = AF_INET; 
    servaddr.sin_port = htons(8181); 
    servaddr.sin_addr.s_addr = INADDR_ANY; 

    char buf[100];

    // Enter the file name from user
    printf("Enter text\n");
    gets(buf);

    sendto(sockfd, buf, strlen(buf) + 1, 0, (const struct sockaddr *) &servaddr, sizeof(servaddr));

    signal(SIGIO, my_recv);


    if (fcntl(sockfd, F_SETOWN, getpid()) < 0){
		perror("fcntl F_SETOWN");
		exit(1);
	}

    int open_flag = fcntl(sockfd, F_GETFL);

	// Make async I/O
	if (fcntl(sockfd, F_SETFL, open_flag | FASYNC) <0 ){
		perror("fcntl F_SETFL, FASYNC");
		exit(1);
	}

    while(1);
}