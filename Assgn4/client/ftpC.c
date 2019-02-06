/*
FTP Client

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
#include <time.h>
#include <signal.h>

#define BUF_SIZE 10
#define PORT_X 50000

int PORT_Y = 55000;

void delay(unsigned int mseconds)
{
    clock_t goal = mseconds + clock();
    while (goal > clock());
}

int main()
{
	int			sockfd ;		// Global sockfd
	struct sockaddr_in	serv_addr, cli_addr;

	int i;
	char buf[100];

	/* Opening a socket is exactly similar to the server process */
	if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
		perror("Unable to create socket\n");
		exit(0);
	}

	// Set server address
	serv_addr.sin_family	= AF_INET;
	inet_aton("127.0.0.1", &serv_addr.sin_addr);
	serv_addr.sin_port	= htons(PORT_X);

	// Connect to server and handshake
	if ((connect(sockfd, (struct sockaddr *) &serv_addr,
						sizeof(serv_addr))) < 0) {
		perror("Unable to connect to server\n");
		exit(0);
	}
	else
		printf("Connection established\n");

	while(1)
	{
		printf("\n> ");
		char command[100];
		gets(command);
		int pos = 0;
		for(;command[pos]==' ';pos++);
		char comm1[100], comm2[100];
		sscanf(command+pos, "%s", comm1);
		pos += strlen(comm1);
		for(;command[pos]==' ';pos++);
		sscanf(command+pos, "%s", comm2);
		if (!strlen(command) || !strlen(comm1))
			continue;
		send(sockfd, command, strlen(command)+1, 0);

		if (strcmp(comm1, "get")==0)	// For get command
		{
			// printf("In get\n");
			pid_t p = fork();
			if (p < 0)
			{
				perror("Unable to fork ");
				exit(EXIT_FAILURE);
			}
			else if (p == 0)	// child
			{
				int sockfd_get;
				struct sockaddr_in	cli_addr_get, serv_addr_get;
				if ((sockfd_get = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
					perror("Cannot create socket\n");
					exit(EXIT_FAILURE);
				}

				serv_addr_get.sin_family		= AF_INET;
				serv_addr_get.sin_addr.s_addr	= INADDR_ANY;
				serv_addr_get.sin_port			= htons(PORT_Y);  // Assign port 20000

				if (setsockopt(sockfd_get, SOL_SOCKET, SO_REUSEADDR, &(int){1}, sizeof(int)) < 0)
				{
    				perror("Unable to reuse socket");
    				exit(EXIT_FAILURE);
				}


				if (bind(sockfd_get, (struct sockaddr *) &serv_addr_get,
								sizeof(serv_addr_get)) < 0) {
					perror("Unable to bind local address");
					exit(0);
				}

				listen(sockfd_get, 5);

				int clilen = sizeof(cli_addr);
				int newsockfd_get = accept(sockfd_get, (struct sockaddr *) &cli_addr,
							&clilen) ;	// blocks server for this client

				// int file = open(comm2, O_CREAT | O_WRONLY | O_TRUNC, S_IRUSR | S_IWUSR);


				int file, FILE_FLAG = 1, byte_count = 0, word_count = 0;
				int read_word = 1, DONE_FLAG = 0;

				file = open(comm2, O_CREAT | O_WRONLY | O_TRUNC, S_IRUSR | S_IWUSR);
				for (i = 1; ; i++)	// use i as a count variable
				{
					short read_bytes;
					char head;


					// printf("Receiving from server\n");
					int n = recv(newsockfd_get, &head, 1, 0);
					if (n < 0)
					{
						perror("Unable to receive");
						exit(EXIT_FAILURE);
					}
					n = recv(newsockfd_get, &read_bytes, sizeof(read_bytes), 0);
					if (n < 0)
					{
						perror("Unable to receive");
						exit(EXIT_FAILURE);
					}

					char buf_temp[BUF_SIZE];
					memset(buf_temp, 0, BUF_SIZE);
					int read_till_here = 0;

					while(1)
					{
						if (read_bytes - read_till_here < BUF_SIZE)
							n = recv(newsockfd_get, buf_temp, read_bytes, 0);
						else
							n = recv(newsockfd_get, buf_temp, BUF_SIZE, 0);


						if (n < 0)
						{
							perror("Unable to receive");
							exit(EXIT_FAILURE);
						}
						
						if (n > 0)
						{
							read_till_here+=n;
							byte_count += n;
						}
						// printf("Received: %s\nnum bytes: %d\n", buf_temp, read_bytes);

						if (n < 0)
						{
							perror("Unable to receive");
							exit(EXIT_FAILURE);
						}	

						// If reading is incomplete, write to file
						if (FILE_FLAG)
							write(file, buf_temp, n);

						if (read_till_here == read_bytes)
							break;
					}

					// Check if socket has been closed
					if ((n == 0 || (n == -1 && errno == EWOULDBLOCK) ) && DONE_FLAG)
					{
						// printf("Reading complete\n");
						close(file);
						close(newsockfd_get);
						FILE_FLAG = 0;
						break;
					}

					if (head == 'L')
						DONE_FLAG = 1;
				}
				close(sockfd_get);
				exit(0);
			}
			else 		// parent
			{
				int return_code[]={0};
				int n = recv(sockfd, return_code, sizeof(int), 0);

				return_code[0] = ntohl(return_code[0]);
				if (n == 0)
				{
					printf("Error: Connection broken\n");
					close(sockfd);
					exit(EXIT_FAILURE);

				}
				// Handling error codes
				printf("Return code received: %d\n", return_code[0]);
				if (return_code[0] == 550)
				{
					printf("Error: Connection error or file could not be read\n");
					kill(p, SIGKILL);
				}
				else if (return_code[0] == 501)
				{
					kill(p, SIGKILL);
					printf("Error: Invalid argument\n");
				}
				else if (return_code[0] == 250)
				{
					int status;
					wait(NULL);
					printf("Success: File transferred\n");
				}
				else if (return_code[0] == 503)
				{
					printf("Error: Port not set\n");
					close(sockfd);
					exit(EXIT_FAILURE);
				}
				else
				{
					close(sockfd);
					exit(EXIT_FAILURE);
				}

			}
		}

		else if (strcmp(command,"quit")==0)
		{
			int return_code[]={0};
			int n = recv(sockfd, return_code, sizeof(int), 0);

			return_code[0] = ntohl(return_code[0]);
			printf("Return code received: %d\n", return_code[0]);
			if (return_code[0] == 421 || return_code[0] == 503 || return_code[0] == 0)
			{
				close(sockfd);
				printf("Success: Server connection closed\n");
				exit(EXIT_SUCCESS);
			}
			else
				continue;
		}


		else if (strcmp(comm1, "put")==0)
		{
			pid_t p = fork();
			if (p < 0)
			{
				perror("Unable to fork ");
				exit(EXIT_FAILURE);
			}
			else if (p == 0)	// child
			{
				int sockfd_put;
				struct sockaddr_in	cli_addr_get, serv_addr_get;
				if ((sockfd_put = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
					perror("Cannot create socket\n");
					exit(EXIT_FAILURE);
				}

				serv_addr_get.sin_family		= AF_INET;
				serv_addr_get.sin_addr.s_addr	= INADDR_ANY;
				serv_addr_get.sin_port			= htons(PORT_Y);  // Assign port 20000

				if (setsockopt(sockfd_put, SOL_SOCKET, SO_REUSEADDR, &(int){1}, sizeof(int)) < 0)
				{
    				perror("Unable to reuse socket");
    				exit(EXIT_FAILURE);
				}

				if (bind(sockfd_put, (struct sockaddr *) &serv_addr_get,
								sizeof(serv_addr_get)) < 0) {
					perror("Unable to bind local address");
					exit(EXIT_FAILURE);
				}

				listen(sockfd_put, 5);

				int clilen = sizeof(cli_addr);
				int newsockfd_put = accept(sockfd_put, (struct sockaddr *) &cli_addr,
							&clilen) ;	// blocks server for this client

				int file = open(comm2, O_RDONLY);

				if (file < 0)
				{
					// File not found or could not be opened
					close(sockfd_put);
					close(newsockfd_put);
					exit(EXIT_FAILURE);
				}


				while(1)
				{
					char buf_temp[BUF_SIZE];
					memset(buf_temp, 0, BUF_SIZE);
					short read_bytes = read(file, buf_temp, BUF_SIZE - 1);

					if (read_bytes <= 0)
					{
						// printf("Reading complete\n");

						if (send(newsockfd_put, "L", 1, 0) < 0)
						{
							perror("Unable to send data");
							exit(EXIT_FAILURE);
						}

						if (send(newsockfd_put, &read_bytes, sizeof(read_bytes), 0)  < 0)
						{
							perror("Unable to send data");
							exit(EXIT_FAILURE);
						}

						if (send(newsockfd_put, "", 0, 0)  < 0)
						{
							perror("Unable to send data");
							exit(EXIT_FAILURE);
						}

						close(file);
						close(newsockfd_put);
						break;
					}

					// find length of buffer read
					int len = strlen(buf_temp);

					// send to server
					if (send(newsockfd_put, "S", 1, 0) < 0)
					{
						perror("Unable to send data");
						exit(EXIT_FAILURE);
					}

					if (send(newsockfd_put, &read_bytes, sizeof(read_bytes), 0)  < 0)
					{
						perror("Unable to send data");
						exit(EXIT_FAILURE);
					}

					if (send(newsockfd_put, buf_temp, read_bytes, 0)  < 0)
					{
						perror("Unable to send data");
						exit(EXIT_FAILURE);
					}
					// printf("Data sent from client\n");
				}

				close(sockfd_put);
				exit(0);
			}
			else 		// parent
			{
				int return_code[]={0};
				int n = recv(sockfd, return_code, sizeof(int), 0);

				return_code[0] = ntohl(return_code[0]);

				printf("Return code received: %d\n", return_code[0]);
				if (n == 0)
				{
					printf("Error: Connection broken\n");

				}
				if (return_code[0] == 550)
				{
					int status;
					printf("Error: File could not be transferred\n");
					kill(p, SIGKILL);
				}
				else if (return_code[0] == 501)
				{
					kill(p, SIGKILL);
					printf("Error: Invalid argument\n");
				}

				else if (return_code[0] == 250)
				{
					int status;
					printf("Success: File transferred\n");
					wait(NULL);
				}
				else if (return_code[0] == 503)
				{
					printf("Error: Port not set\n");
					close(sockfd);
					exit(EXIT_FAILURE);
				}
				else
				{
					close(sockfd);
					exit(EXIT_FAILURE);
				}

			}

		}

		else if (strcmp(comm1, "cd")==0)
		{
			int return_code[]={0};
			int n = recv(sockfd, return_code, sizeof(int), 0);

			return_code[0] = ntohl(return_code[0]);

			printf("Return code received: %d\n", return_code[0]);
			if (n == 0)
				printf("Error: Connection broken\n");
			if (return_code[0] == 200)
			{
				int status;
				printf("Success: Directory changed\n");
			}
			else if (return_code[0] == 501)
			{
				int status;
				printf("Error: Invalid argument or directory change unsuccessful\n");
			}

			else if (return_code[0] == 503)
			{
				printf("Error: Port not set\n");
				close(sockfd);
				exit(EXIT_FAILURE);
			}
			else
			{
				close(sockfd);
				exit(EXIT_FAILURE);
			}
		}

		else if (strcmp(comm1, "port")==0)  // For port
		{
			// printf("sent port\n");
			int return_code[]={0};
			int n = recv(sockfd, return_code, sizeof(int), 0);

			return_code[0] = ntohl(return_code[0]);
			printf("Return code received: %d\n", return_code[0]);
			if (return_code[0] == 550)
			{
				printf("Error: Invalid port\n");
				close(sockfd);
				exit(EXIT_FAILURE);
			}
			if (return_code[0] == 0)
			{
				printf("Error: Connection broken\n");
				close(sockfd);
				exit(EXIT_FAILURE);
			}
			if (return_code[0] == 501)
				printf("Error: Invalid argument\n");
			else if (return_code[0] != 200)
			{
				printf("Error: closing all connections\n");
				close(sockfd);
				exit(EXIT_FAILURE);
			}
			else
			{
				printf("Success: Port accepted by server\n");
				PORT_Y = atoi(comm2);
			}
		}
		else
		{
			int return_code[]={0};
			int n = recv(sockfd, return_code, sizeof(int), 0);

			return_code[0] = ntohl(return_code[0]);

			printf("Return code received: %d\n", return_code[0]);
			if (return_code[0] == 502)
			{
				printf("Error: command not found\n");
			}
			else if (return_code[0] == 501)
				printf("Error: Invalid argument\n");
			else if (return_code[0] == 503)
			{
				printf("Error: Port not set\n");
				close(sockfd);
				exit(EXIT_FAILURE);
			}
			else if (return_code[0] == 0)
			{
				printf("Error: Connection broken\n");
				close(sockfd);
				exit(EXIT_FAILURE);
			}
		}
	}
}


