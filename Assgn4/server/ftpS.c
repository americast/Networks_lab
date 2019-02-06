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

#define BUF_SIZE 10
#define PORT_X 50000

int PORT_Y = 55000;


int main()
{
	int			sockfd, newsockfd, sockfd_get, sockfd_put ; /* Socket descriptors */
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

	if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &(int){1}, sizeof(int)) < 0)
	{
		perror("Could not reuse port");
		exit(EXIT_FAILURE);
	}

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
		int FIRST_FLAG = 1;

		while(1)
		{
			char command[100], comm1[100], comm2[100];
			printf("\nWaiting for command from client\n"); 
		
			// Receive filename from client
			int n = recv(newsockfd, command, 100, 0);
			if (!n)
			{
				close(newsockfd);
				FIRST_FLAG = 1;
				break;
			}
			printf("Received data from client\n");
			int pos = 0;
			for(;command[pos]==' ';pos++);
			sscanf(command+pos, "%s", comm1);
			pos += strlen(comm1);
			for(;command[pos]==' ';pos++);
			sscanf(command+pos, "%s", comm2);

			if (FIRST_FLAG)
			{
				if (strcmp(comm1, "port")==0)
				{
					PORT_Y = atoi(comm2);
					if (PORT_Y > 1024 && PORT_Y < 65535)
					{
						printf("Valid port\n");
						return_code[0] = htonl(200);
						send(newsockfd, return_code, sizeof(int), 0);
						FIRST_FLAG = 0;
						continue;
					}
					else
					{
						printf("Invalid port\n");
						return_code[0] = htonl(550);
						send(newsockfd, return_code, sizeof(int), 0);
						close(newsockfd);
						break;
					}
				}
				else
				{
					printf("Port not set\n");
					return_code[0] = htonl(503);
					send(newsockfd, return_code, sizeof(int), 0);
					close(newsockfd);
					break;
				}
			}
			
			if (strcmp(comm1, "get")==0)
			{
				printf("In get\n");
				printf("File to open %s\n", comm2);
				int file = open(comm2, O_RDONLY);

				if (file < 0)
				{
					return_code[0] = htonl(550);
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
							perror("Unable to create socket");
							// return_code[0] = htonl(550);
							// send(newsockfd, return_code, sizeof(int), 0);
							exit(EXIT_FAILURE);
						}

						serv_addr.sin_port	= htons(PORT_Y);

						if ((connect(sockfd_get, (struct sockaddr *) &serv_addr,
												sizeof(serv_addr))) < 0) {
								perror("Unable to connect to server\n");
								exit(EXIT_FAILURE);
						}
						else
							printf("Connection established\n");

						while(1)
						{
							char buf_temp[BUF_SIZE];
							memset(buf_temp, 0, BUF_SIZE);
							printf("Reading from file\n");
							short read_bytes = read(file, buf_temp, BUF_SIZE - 1);
							printf("Read: %s\n", buf_temp);
							// Read from file complete
							if (read_bytes <= 0)
							{
								printf("Reading complete\n");

								if (send(sockfd_get, "L", 1, 0) < 0)
								{
									perror("Unable to send data");
									exit(EXIT_FAILURE);
								}
								if (send(sockfd_get, &read_bytes, sizeof(read_bytes), 0) < 0)
								{
									perror("Unable to send data");
									exit(EXIT_FAILURE);
								}
								
								// send to client
								if (send(sockfd_get, "", 0, 0) < 0)
								{
									perror("Unable to send data");
									exit(EXIT_FAILURE);
								}


								close(file);
								close(sockfd_get);
								break;
							}

							// find length of buffer read
							int len = strlen(buf_temp);
							printf("len: %d\n", len);
							printf("Here 1\n");
							if (send(sockfd_get, "S", 1, 0) < 0)
							{
								perror("Unable to send data");
								exit(EXIT_FAILURE);
							}
							printf("Here 2\n");
							if (send(sockfd_get, &read_bytes, sizeof(read_bytes), 0) < 0)
							{
								perror("Unable to send data");
								exit(EXIT_FAILURE);
							}
							
							// send to client
							printf("Here 3\n");
							if (send(sockfd_get, buf_temp, read_bytes, 0) < 0)
							{
								perror("Unable to send data");
								exit(EXIT_FAILURE);
							}
							printf("Data sent from server\n");
						}

						close(sockfd_get);
						exit(EXIT_SUCCESS);
						// return_code[0] = htonl(250);
						// send(newsockfd, return_code, sizeof(int), 0);

					}
					else // parent
					{
						int return_code[] = {0};
						int stat;
						wait(&stat);
						
						stat = WEXITSTATUS(stat);
						if (stat == EXIT_FAILURE)
						{
							return_code[0] = htonl(550);
							send(newsockfd, return_code, sizeof(int), 0);
						}
						else
						{
							return_code[0] = htonl(250);
							send(newsockfd, return_code, sizeof(int), 0);
						}
					}
				}
			}






			else if (strcmp(comm1, "put")==0)
			{
				printf("In put\n");
				printf("File to open %s\n", comm2);
				int return_code[] = {0};

				
				pid_t p = fork();
				if (p < 0)
				{
					perror("Could not fork");
					exit(EXIT_FAILURE);
				}
				else if (p == 0)	// child
				{
					/* Opening a socket is exactly similar to the server process */
					if ((sockfd_put = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
						perror("Unable to create socket\n");
						// return_code[0] = htonl(550);
						// send(newsockfd, return_code, sizeof(int), 0);
						exit(EXIT_FAILURE);
					}

					serv_addr.sin_port	= htons(PORT_Y);

					if ((connect(sockfd_put, (struct sockaddr *) &serv_addr,
											sizeof(serv_addr))) < 0) {
							perror("Unable to connect to server\n");
							exit(EXIT_FAILURE);
					}
					else
						printf("Connection established\n");

					
					int file, FILE_FLAG = 1, byte_count = 0, word_count = 0;
					int read_word = 1, DONE_FLAG = 0;

					for (i = 1; ; i++)	// use i as a count variable
					{
						char head;
						short read_bytes;


						printf("Receiving from client\n");
						if (recv(sockfd_put, &head, 1, 0) < 0)
						{
							perror("Could not receive");
							exit(EXIT_FAILURE);
						}
						printf("Head received %c\n", head);

						if (recv(sockfd_put, &read_bytes, sizeof(read_bytes), 0) < 0)
						{
							perror("Could not receive");
							exit(EXIT_FAILURE);
						}

						printf("Received read_bytes: %d\n", read_bytes);

						char buf_temp[read_bytes];
						memset(buf_temp, 0, read_bytes);

						int n = recv(sockfd_put, buf_temp, read_bytes, 0);
						
						if (n < 0)
						{
							perror("Could not receive");
							exit(EXIT_FAILURE);
						}
						if (n > 0)
							byte_count += n;


						printf("Received: %s\nnum bytes: %hu\n", buf_temp, read_bytes);

						if (n < 0)
						{
							perror("Some error occured");
							exit(EXIT_FAILURE);
						}
						// Check if socket has been closed
						if (i == 1 && n == 0)
						{
							printf("File not found.\n");
							close(sockfd_put);
							// return_code[0] = htonl(550);
							// send(newsockfd, return_code, sizeof(int), 0);
							exit(EXIT_FAILURE);
						}
						else if (i == 1)
						{
							file = open(comm2, O_CREAT | O_WRONLY | O_TRUNC, S_IRUSR | S_IWUSR);
							if (file < 0)
							{
								// return_code[0] = htonl(550);
								// send(newsockfd, return_code, sizeof(int), 0);
								// break;
								exit(EXIT_FAILURE);
							}
						}
						if ((n == 0 || (n == -1 && errno == EWOULDBLOCK)) && DONE_FLAG)
						{
							printf("Reading complete\n");
							close(file);
							close(sockfd_put);
							FILE_FLAG = 0;
							exit(EXIT_SUCCESS);
						}

						if (head == 'L')
							DONE_FLAG = 1;

						// If reading is incomplete, write to file
						if (FILE_FLAG)
						{
							printf("Writing to file\n");
							printf("len is %d\n", strlen(buf_temp));
							write(file, buf_temp, read_bytes);
						}

					}

					exit(EXIT_SUCCESS);
					// close(sockfd_put);
					// printf("Return code is: %d\n", return_code[0]);
					// if (return_code[0] != htonl(550))
					// {
					// 	return_code[0] = htonl(250);
					// 	send(newsockfd, return_code, sizeof(int), 0);						
					// }
				}
				else // parent
				{
					int stat, return_code[] = {0};
					wait(&stat);
					stat = WEXITSTATUS(stat);
					printf("Return status: %d\n", stat);
					if (stat == EXIT_FAILURE)
					{
						return_code[0] = htonl(550);
						send(newsockfd, return_code, sizeof(int), 0);
					}
					else
					{
						return_code[0] = htonl(250);
						send(newsockfd, return_code, sizeof(int), 0);

					}
				}
				
			}

			else if (strcmp(comm1, "cd")==0)
			{
				int n = chdir(comm2);
				if (n < 0)
				{
					perror("Some error occured");
					return_code[0] = htonl(501);
					send(newsockfd, return_code, sizeof(int), 0);						
				}
				else
				{
					return_code[0] = htonl(200);
					send(newsockfd, return_code, sizeof(int), 0);	
				}
			}

			else if (strcmp(comm1, "quit")==0)
			{
				return_code[0] = htonl(421);
				send(newsockfd, return_code, sizeof(int), 0);
				printf("Closing Connection\n");	
				close(newsockfd);
				FIRST_FLAG = 1;
				break;
			}

			else
			{
				return_code[0] = htonl(502);
				send(newsockfd, return_code, sizeof(int), 0);
				printf("Closing Connection\n");	
				continue;
			}
		}
	}
}
			
