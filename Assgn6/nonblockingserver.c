/*

Server

16CS10048
Sayan Sinha

*/

#include <stdio.h>
#include <fcntl.h> 
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h> 
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <errno.h>
#define PORT_UDP 8001
#define PORT_TCP 8002
#define BUF_SIZE 100


int main()
{
	int	sockfd_udp, sockfd_tcp, newsockfd;  // define sockets
	int	clilen;
	struct sockaddr_in	serv_addr, cli_addr;			// server address details

	int i;
	char buf[100];	

	if ((sockfd_udp = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {	// Create UDP socket
		printf("Cannot create socket\n");
		exit(EXIT_FAILURE);
	}

	if ((sockfd_tcp = socket(AF_INET, SOCK_STREAM, 0)) < 0) {	// Create TCP socket
	 perror("Cannot create socket\n");
	 exit(EXIT_FAILURE);
	}

	serv_addr.sin_family		= AF_INET;
	serv_addr.sin_addr.s_addr	= INADDR_ANY;
	serv_addr.sin_port		= htons(PORT_UDP);	// Set server address for UDP

	if (setsockopt(sockfd_udp, SOL_SOCKET, SO_REUSEADDR, &(int){1}, sizeof(int)) < 0)
	{
		perror("Could not reuse port");
		exit(EXIT_FAILURE);
	}

	if (bind(sockfd_udp, (struct sockaddr *) &serv_addr,  // bind the UDP server
					sizeof(serv_addr)) < 0) {
		perror("Unable to bind local address ");
		exit(0);
	}


	serv_addr.sin_port		= htons(PORT_TCP);  // Set server address for TCP

	if (setsockopt(sockfd_tcp, SOL_SOCKET, SO_REUSEADDR, &(int){1}, sizeof(int)) < 0)
	{
		perror("Could not reuse port");
		exit(EXIT_FAILURE);
	}

	if (bind(sockfd_tcp, (struct sockaddr *) &serv_addr,  // bind the server for TCP
					sizeof(serv_addr)) < 0) {
		perror("Unable to bind local address ");
		exit(0);
	}

	if (fcntl(sockfd_tcp, F_SETFL, O_NONBLOCK) < 0)
	{
		perror("Unable to switch to nonblocking");
		exit(EXIT_FAILURE);
	}

	listen(sockfd_tcp, 5); 

	fd_set sock;


	while (1)
	{
		usleep(1000);
		printf("Waiting for client\n");
		clilen = sizeof(cli_addr);

		for(i=0; i < 100; i++) buf[i] = '\0'; // Set to all zeros

		while(1)
		{
			printf("Waiting for UDP client\n");
			int n = recvfrom(sockfd_udp, buf, 100, MSG_DONTWAIT, (const struct sockaddr *) &cli_addr, &clilen);  // Non blocking recv call

			if (n == -1 && (errno == EWOULDBLOCK || errno == EAGAIN))
				break;

			// if (udp_avl)	// If UDP available
			{
				pid_t p = fork();
				if (p < 0)
				{
					perror("Forking failed ");
					continue;
				}
				if (p == 0) {				// Create child process
					
					printf("%s\n", buf);

					struct hostent *lh = gethostbyname(buf);  // Get IP addresses

					char *IPbuffer; 

					int i;
					if (lh)
					{
						for (i = 0; i < lh->h_length; i++)
						{
							if (lh->h_addr_list[i] == NULL)
								break;
							IPbuffer = inet_ntoa(*((struct in_addr*) 
						                           lh->h_addr_list[i])); 
							sendto(sockfd_udp, IPbuffer, strlen(IPbuffer) + 1, 0, (const struct sockaddr *) &cli_addr, sizeof(cli_addr));  // Send received IP address to client
						}
					}
					printf("\nAll IPs have been sent\n");
					sendto(sockfd_udp, "", 0, 0, (const struct sockaddr *) &cli_addr, sizeof(cli_addr));  // Send blank message to mark end of sending

					exit(0);  // Close child process
				}
			}
		}

		while (1)  // If TCP available
		{
			printf("Waiting for TCP client\n");
			newsockfd = accept(sockfd_tcp, (struct sockaddr *) &cli_addr,
								&clilen) ;	// blocks server for this client

			if (newsockfd < 0 && (errno == EWOULDBLOCK || errno == EAGAIN))
				break;				
			
			pid_t p = fork();
			if (p < 0)
			{
				perror("Failed to fork ");
				continue;
			}
			if (p==0)  // Child process
			{
				char req[100];
				recv(newsockfd, req, 100, 0);
				if (strcmp(req, "handshake"))
					exit(EXIT_FAILURE);
				printf("Opening file word.txt\n");

				FILE *fp = fopen("word.txt", "r");  // Open file using pointer

				if (!fp)
				{
					perror("Unable to open file ");
					send(newsockfd, "\0", 2, 0);
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
						fscanf(fp, "%s", buf_temp);

						// Reading from file complete
						if (strlen(buf_temp)==0)
						{
							printf("Reading complete\n");
							send(newsockfd, "\0", 2, 0);
							fclose(fp);			// Close file pointer
							close(newsockfd);	// Close connection
							break;
						}

						// find length of buffer read
						int len = strlen(buf_temp);

						// send to client
						send(newsockfd, buf_temp, strlen(buf_temp)+1, 0);
						printf("Data sent from server\n");
					}
					close(newsockfd);	// Close TCP connection
					exit(EXIT_SUCCESS);
				}
			}
		}
	}
	close(sockfd_udp);
}
			

