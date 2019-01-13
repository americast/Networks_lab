
/*    THE CLIENT PROCESS */


#include <stdio.h> 
#include <fcntl.h> 
#include <errno.h> 
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <time.h>

void delay(unsigned int mseconds)
{
    clock_t goal = mseconds + clock();
    while (goal > clock());
}

int main()
{
	int			sockfd ;
	struct sockaddr_in	serv_addr;

	int i;
	char buf[100];

	/* Opening a socket is exactly similar to the server process */
	if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
		perror("Unable to create socket\n");
		exit(0);
	}

	/* Recall that we specified INADDR_ANY when we specified the server
	   address in the server. Since the client can run on a different
	   machine, we must specify the IP address of the server. 

	   In this program, we assume that the server is running on the
	   same machine as the client. 127.0.0.1 is a special address
	   for "localhost" (this machine)
	   
	/* IF YOUR SERVER RUNS ON SOME OTHER MACHINE, YOU MUST CHANGE 
           THE IP ADDRESS SPECIFIED BELOW TO THE IP ADDRESS OF THE 
           MACHINE WHERE YOU ARE RUNNING THE SERVER. 
    	*/

	serv_addr.sin_family	= AF_INET;
	inet_aton("127.0.0.1", &serv_addr.sin_addr);
	serv_addr.sin_port	= htons(20000);

	/* With the information specified in serv_addr, the connect()
	   system call establishes a connection with the server process.
	*/
	if ((connect(sockfd, (struct sockaddr *) &serv_addr,
						sizeof(serv_addr))) < 0) {
		perror("Unable to connect to server\n");
		exit(0);
	}
	else
		printf("Connection established\n");

	/* After connection, the client can send or receive messages.
	   However, please note that recv() will block when the
	   server is not sending and vice versa. Similarly send() will
	   block when the server is not receiving and vice versa. For
	   non-blocking modes, refer to the online man pages.
	*/

	printf("Enter file name: ");
	gets(buf);

	send(sockfd, buf, strlen(buf) + 1, 0);
	printf("Sent filename %s from client\n", buf);
	


	delay(1000000);
	int file, FILE_FLAG = 0;
	int count_bytes = 0, count_words = 0;

	for (i = 1; ; i++)
	{
		char buf_temp[6];
		memset(buf_temp, 0, 6);
    	fcntl(sockfd, F_SETFL, O_NONBLOCK); /* Change the socket into non-blocking state	*/
		printf("Receiving from server\n");
		int n = recv(sockfd, buf_temp, 5, 0);
		printf("Received: %d\n",n);
		if (i == 1)
		{
			if (n == -1 && errno == EWOULDBLOCK)
			{
				printf("Here1\n");
				printf("File not found.\n");
				break;
			}
			else
			{
				printf("Here2\n");
				file = open("out", O_CREAT | O_WRONLY);
				FILE_FLAG = 1;
			}
		}
		else
		{
			if (n == 0 || (n == -1 && errno == EWOULDBLOCK))
			{
				printf("Here3\n");
				printf("Reading complete\n");
				close(file);
				FILE_FLAG = 0;
				break;
			}
		}
		if (FILE_FLAG)
		{
			printf("Here4\n");
			write(file, buf_temp, strlen(buf_temp)+1);
		}

	}
	// printf("%s\n", buf);

	
	close(sockfd);
}


