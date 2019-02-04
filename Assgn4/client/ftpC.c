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

#define BUF_SIZE 6
#define PORT_X 50000
#define PORT_Y 55000

void delay(unsigned int mseconds)
{
    clock_t goal = mseconds + clock();
    while (goal > clock());
}

int main()
{
	int			sockfd ;
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

		send(sockfd, command, strlen(command)+1, 0);
		if (strcmp(command,"quit")==0)
		{
			close(sockfd);
			break;
		}

		char comm1[100], comm2[100];
		sscanf(command, "%s", comm1);
		int pos = strlen(comm1);
		for(;command[pos]==' ';pos++);
		sscanf(command+pos, "%s", comm2);

		if (strcmp(comm1, "get")==0)
		{
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
					exit(0);
				}

				serv_addr_get.sin_family		= AF_INET;
				serv_addr_get.sin_addr.s_addr	= INADDR_ANY;
				serv_addr_get.sin_port			= htons(PORT_Y);  // Assign port 20000

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
				int read_word = 1;

				file = open(comm2, O_CREAT | O_WRONLY | O_TRUNC, S_IRUSR | S_IWUSR);
				for (i = 1; ; i++)	// use i as a count variable
				{
					char buf_temp[BUF_SIZE+1];
					memset(buf_temp, 0, BUF_SIZE+1);

					buf_temp[BUF_SIZE] = '\0';

					printf("Receiving from server\n");
					int n = recv(newsockfd_get, buf_temp, BUF_SIZE, 0);
					if (n > 0)
						byte_count += n;

					printf("Received: %s\nbytes: %d\n", buf_temp, n);

					if (n < 0)
						perror("Some error occured");
					// Check if socket has been closed
					if (n == 0 || (n == -1 && errno == EWOULDBLOCK))
					{
						printf("Reading complete\n");
						close(file);
						close(newsockfd_get);
						FILE_FLAG = 0;
						break;
					}

					// If reading is incomplete, write to file
					if (FILE_FLAG)
					{
						printf("Writing to file\n");
						write(file, buf_temp, strlen(buf_temp)+1);
					}

				}
				exit(0);
			}
			else 		// parent
			{
				int return_code[]={0};
				int n = recv(sockfd, return_code, sizeof(int), 0);
				if (n == 0)
				{
					printf("Connection broken\n");

				}
				if (return_code[0] == 550)
				{
					int status;
					printf("File not found\n");
					kill(p, SIGKILL);
				}
				else if (return_code[0] == 250)
				{
					int status;
					printf("File transfer successful\n");
					kill(p, SIGTERM);
				}

			}
		}






		if (strcmp(comm1, "put")==0)
		{
			printf("In put\n");
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
					exit(0);
				}

				serv_addr_get.sin_family		= AF_INET;
				serv_addr_get.sin_addr.s_addr	= INADDR_ANY;
				serv_addr_get.sin_port			= htons(PORT_Y);  // Assign port 20000

				if (bind(sockfd_put, (struct sockaddr *) &serv_addr_get,
								sizeof(serv_addr_get)) < 0) {
					perror("Unable to bind local address");
					exit(0);
				}

				listen(sockfd_put, 5);

				int clilen = sizeof(cli_addr);
				int newsockfd_put = accept(sockfd_put, (struct sockaddr *) &cli_addr,
							&clilen) ;	// blocks server for this client

				int file = open(comm2, O_RDONLY);

				if (file < 0)
				{
					printf("File not found\n");
					exit(EXIT_FAILURE);
				}


				while(1)
				{
					char buf_temp[BUF_SIZE];
					memset(buf_temp, 0, BUF_SIZE);
					printf("Reading from file\n");
					int read_bytes = read(file, buf_temp, BUF_SIZE - 1);
					buf_temp[BUF_SIZE] = '\0';
					printf("Read: %s\n", buf_temp);
					// Read from file complete
					if (read_bytes <= 0)
					{
						printf("Reading complete\n");
						close(file);
						close(sockfd_put);
						break;
					}

					// find length of buffer read
					int len = strlen(buf_temp);
					printf("len: %d\n", len);
					// send to server

					send(newsockfd_put, buf_temp, strlen(buf_temp), 0);
					printf("Data sent from client\n");
				}

				close(sockfd_put);
				exit(0);
			}
			else 		// parent
			{
				int return_code[]={0};
				int n = recv(sockfd, return_code, sizeof(int), 0);
				if (n == 0)
				{
					printf("Connection broken\n");

				}
				if (return_code[0] == 550)
				{
					int status;
					printf("Some error occured\n");
					kill(p, SIGKILL);
				}
				else if (return_code[0] == 250)
				{
					int status;
					printf("File transfer successfull\n");
					kill(p, SIGTERM);
				}

			}

		}
	}
}


