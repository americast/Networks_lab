/*
TCP Server

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



int main()
{
	int			sockfd, newsockfd ; /* Socket descriptors */
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
	serv_addr.sin_port			= htons(20000);  // Assign port 20000

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


		printf("\nWaiting for filename from client\n"); 
		
		// Receive filename from client
		recv(newsockfd, buf, 100, 0);
		printf("Opening file %s\n", buf);

		// Open filename received from client
		int file = open(buf, O_RDONLY);

		if (file == -1)
		{
			perror("Unable to open file ");
			close(newsockfd);
			continue;
		}
		else
		{
			printf("File opened.\n");

			while(1)
			{
				char buf_temp[BUF_SIZE];
				memset(buf_temp, 0, BUF_SIZE);
				printf("Reading from file\n");
				int read_bytes = read(file, buf_temp, BUF_SIZE - 1);

				// Read from file complete
				if (read_bytes <= 0)
				{
					printf("Reading complete\n");
					close(file);
					close(newsockfd);
					break;
				}

				// find length of buffer read
				int len = strlen(buf_temp);

				// send to client
				send(newsockfd, buf_temp, strlen(buf_temp), 0);
				printf("Data sent from server\n");
			}
			continue;
		}

		// Close the socket
		close(newsockfd);
	}
}
			
