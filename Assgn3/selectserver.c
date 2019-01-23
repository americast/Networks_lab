/*
			NETWORK PROGRAMMING WITH SOCKETS

In this program we illustrate the use of Berkeley sockets for interprocess
communication across the network. We show the communication between a server
process and a client process.


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
#define PORT_UDP 8000
#define PORT_TCP 8000
#define BUF_SIZE 6

			/* THE SERVER PROCESS */

main()
{
	int			sockfd_udp, sockfd_tcp, newsockfd ; /* Socket descriptors */
	int			clilen;
	struct sockaddr_in	serv_addr;

	int i;
	char buf[100];		/* We will use this buffer for communication */

	/* The following system call opens a socket. The first parameter
	   indicates the family of the protocol to be followed. For internet
	   protocols we use AF_INET. For TCP sockets the second parameter
	   is SOCK_STREAM. The third parameter is set to 0 for user
	   applications.
	*/
	if ((sockfd_udp = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
		printf("Cannot create socket\n");
		exit(0);
	}

	if ((sockfd_tcp = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
	 perror("Cannot create socket\n");
	 exit(0);
	}

	// listen(sockfd_tcp, 5);

	/* The structure "sockaddr_in" is defined in <netinet/in.h> for the
	   internet family of protocols. This has three main fields. The
 	   field "sin_family" specifies the family and is therefore AF_INET
	   for the internet family. The field "sin_addr" specifies the
	   internet address of the server. This field is set to INADDR_ANY
	   for machines having a single IP address. The field "sin_port"
	   specifies the port number of the server.
	*/

	serv_addr.sin_family		= AF_INET;
	serv_addr.sin_addr.s_addr	= INADDR_ANY;
	serv_addr.sin_port		= htons(PORT_UDP);

	/* With the information provided in serv_addr, we associate the server
	   with its port using the bind() system call. 
	*/
	if (bind(sockfd_udp, (struct sockaddr *) &serv_addr,
					sizeof(serv_addr)) < 0) {
		perror("Unable to bind local address ");
		exit(0);
	}
	serv_addr.sin_port		= htons(PORT_TCP);

	if (bind(sockfd_tcp, (struct sockaddr *) &serv_addr,
					sizeof(serv_addr)) < 0) {
		perror("Unable to bind local address ");
		exit(0);
	}
	listen(sockfd_tcp, 5); 
	/* This specifies that up to 5 concurrent client
			      requests will be queued up while the system is
			      executing the "accept" system call below.
			   */

	/* In this program we are illustrating a concurrent server -- one
	   which forks to accept multiple client connections concurrently.
	   As soon as the server accepts a connection from a client, it
	   forks a child which communicates with the client, while the
	   parent becomes free to accept a new connection. To facilitate
	   this, the accept() system call returns a new socket descriptor
	   which can be used by the child. The parent continues with the
	   original socket descriptor.
	*/
	fd_set sock;
	while (1)
	{
		FD_ZERO(&sock);
		FD_SET(sockfd_udp, &sock); 
		FD_SET(sockfd_tcp, &sock); 

		printf("Waiting for client\n");
		int selected = select(sockfd_udp > sockfd_tcp ? sockfd_udp + 1 : sockfd_tcp + 1, &sock, NULL, NULL, NULL);
		struct sockaddr_in	cli_addr;
		clilen = sizeof(cli_addr);


			for(i=0; i < 100; i++) buf[i] = '\0';
		if (FD_ISSET(sockfd_udp, &sock))
		{
			if (fork() == 0) {
				printf("Waiting for UDP client\n");
				recvfrom(sockfd_udp, buf, 100, 0, (const struct sockaddr *) &cli_addr, &clilen);
				printf("%s\n", buf);

				struct hostent *lh = gethostbyname(buf);

				char *IPbuffer; 

				int i;

				char all[] = "";

				// printf("Length: %d\n", lh->h_length);
				for (i = 0; i < lh->h_length; i++)
				{

					// printf("Here1\n");
					// printf("Here[0]: %d\n", (int)lh->h_addr_list[i][0]);
					// if (strlen(lh->h_addr_list[i]) == 0)
					// 	break;
					// printf("Here2\n");
					if (lh->h_addr_list[i] == NULL)
						break;
					IPbuffer = inet_ntoa(*((struct in_addr*) 
				                           lh->h_addr_list[i])); 
					// printf("Here3\n");

					printf("IP buffer %d: %s\n", i+1, (IPbuffer));
					strcat(all, IPbuffer);
					strcat(all, " ");
				}
				sendto(sockfd_udp, all, strlen(all) + 1, 0, (const struct sockaddr *) &cli_addr, sizeof(cli_addr));
				printf("\nAll IPs have been displayed\n");

				// close(sockfd_udp);
				exit(0);
			}
		}
		if (FD_ISSET(sockfd_tcp, &sock))
		{
			if (fork()==0)
			{
				printf("Waiting for TCP client\n");
				newsockfd = accept(sockfd_tcp, (struct sockaddr *) &cli_addr,
									&clilen) ;	// blocks server for this client

				if (newsockfd < 0) {
					perror("Accept error");
					exit(0);
				}
				else
					printf("Connection established\n");

				printf("Opening file word.txt\n", buf);

				// Open filename received from client
				int file = open("word.txt", O_RDONLY);

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
							send(newsockfd, "\0", 1, 0);
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
					close(newsockfd);
					return 0;
				}

				// Close the socket
			}
		}
		
		
		// wait(NULL);
		// close(sockfd_udp);
	}
}
			

