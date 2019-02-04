/*
FTP Server

Sayan Sinha
16CS10048
*/

#include <stdio.h> 
#include <fcntl.h> 
#include <errno.h> 
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h> 
#include <netinet/in.h>
#include <arpa/inet.h>

#define BUF_SIZE 3
#define PORT_X 50000
#define PORT_Y 55000


int main()
{
	int			sockfd, newsockfd, sockfd_get ; /* Socket descriptors */
	int			clilen;
	struct sockaddr_in	cli_addr, serv_addr;

	int i;
	char buf[100];		/* We will use this buffer for communication */

	if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
		perror("Cannot create socket\n");
		exit(0);
	}

	serv_addr.sin_family		= AF_INET;
	serv_addr.sin_addr.s_addr	= INADDR_ANY;
	serv_addr.sin_port			= htons(PORT_X);  // Assign port 20000

	if (bind(sockfd, (struct sockaddr *) &serv_addr,
					sizeof(serv_addr)) < 0) {
		perror("Unable to bind local address");
		exit(0);
	}

	listen(sockfd, 5);	// upto 5 concurrent clients can be handled

	// Always on server
	while (1)
	{

		clilen = sizeof(cli_addr);
		newsockfd = accept(sockfd, (struct sockaddr *) &cli_addr,
					&clilen) ;	// blocks server for this client

		if (newsockfd < 0) {
			perror("Accept error");
			exit(0);
		}
		else
			printf("Connection established\n");

		int return_code[]={0};

		while(1)
		{
			char command[100], comm1[100], comm2[100];
			printf("\nWaiting for client\n"); 
		
			// Receive filename from client
			int n = recv(newsockfd, command, 100, 0);
			if (!n)
			{
				close(newsockfd);
				break;
			}
			printf("Received data from client\n");
			sscanf(command, "%s", comm1);
			int pos = strlen(comm1);
			for(;command[pos]==' ';pos++);
			sscanf(command+pos, "%s", comm2);
			
			if (strcmp(comm1, "get")==0)
			{
				printf("In get\n");
				printf("File to open %s\n", comm2);
				int file = open(comm2, O_RDONLY);

				if (file < 0)
				{
					return_code[0] = 550;
					send(newsockfd, return_code, sizeof(int), 0);
				}
				else
				{
					pid_t p = fork();
					if (p < 0)
					{
						perror("Could not fork");
						exit(EXIT_FAILURE);
					}
					else if (p == 0)	// child
					{
						/* Opening a socket is exactly similar to the server process */
						if ((sockfd_get = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
							perror("Unable to create socket\n");
							exit(0);
						}

						serv_addr.sin_port	= htons(PORT_Y);

						if ((connect(sockfd_get, (struct sockaddr *) &serv_addr,
												sizeof(serv_addr))) < 0) {
								perror("Unable to connect to server\n");
								exit(0);
						}
						else
							printf("Connection established\n");

						while(1)
						{
							char buf_temp[BUF_SIZE];
							memset(buf_temp, 0, BUF_SIZE);
							printf("Reading from file\n");
							int read_bytes = read(file, buf_temp, BUF_SIZE - 1);
							printf("Read: %s\n", buf_temp);
							// Read from file complete
							if (read_bytes <= 0)
							{
								printf("Reading complete\n");
								close(file);
								close(sockfd_get);
								break;
							}

							// find length of buffer read
							int len = strlen(buf_temp);
							printf("len: %d\n", len);
							// send to client
							send(sockfd_get, buf_temp, strlen(buf_temp), 0);
							printf("Data sent from server\n");
						}

						close(sockfd_get);
						return_code[0] = 250;
						send(newsockfd, return_code, sizeof(int), 0);

					}
						
				}
			}
		}
	}
}
			
