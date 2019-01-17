/*
TCP Client

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

#define BUF_SIZE 6

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

	// Set server address
	serv_addr.sin_family	= AF_INET;
	inet_aton("127.0.0.1", &serv_addr.sin_addr);
	serv_addr.sin_port	= htons(20000);

	// Connect to server and handshake
	if ((connect(sockfd, (struct sockaddr *) &serv_addr,
						sizeof(serv_addr))) < 0) {
		perror("Unable to connect to server\n");
		exit(0);
	}
	else
		printf("Connection established\n");

	// Take input from user about filename
	printf("Enter file name: ");
	gets(buf);

	// Send filename to server
	send(sockfd, buf, strlen(buf) + 1, 0);
	printf("Sent filename %s from client\n", buf);
	
	// Delay ensures server gets enough time to send data to pipe
	delay(100000);
	int file, FILE_FLAG = 0, byte_count = 0, word_count = 0;
	int read_word = 1;

	for (i = 1; ; i++)	// use i as a count variable
	{
		char buf_temp[BUF_SIZE+1];
		memset(buf_temp, 0, BUF_SIZE+1);

		buf_temp[BUF_SIZE] = '\0';

		printf("Receiving from server\n");
		int n = recv(sockfd, buf_temp, BUF_SIZE, 0);
		if (n > 0)
		{
			int i;
			byte_count += n;
			for (i = 0; i < n; i++)
			{
				// check presence of a delimiter
				if (buf_temp[i] == ',' || buf_temp[i] == ';' || buf_temp[i] == ':' || buf_temp[i] == '.' || buf_temp[i] == ' ' || buf_temp[i] == '\t')
				{
					read_word = 1;
					continue;
				}
				if (read_word)
				{
					word_count+=1;
					read_word = 0;
				}
			}
		}

		// Check if socket has been closed
		if (i == 1)
		{
			if (n == 0 || (n == -1 && errno == EWOULDBLOCK))
			{
				printf("File not found.\n");
				break;
			}
			else
			{
				file = open(buf, O_CREAT | O_WRONLY | O_TRUNC, S_IRUSR | S_IWUSR);
				if (file < 0)
				{
					perror("Error opening file ");
					return 1;
				}
				FILE_FLAG = 1;
			}
		}
		// Check if reading is complete
		else
		{
			if (n == 0 || (n == -1 && errno == EWOULDBLOCK))
			{
				printf("Reading complete\n");
				close(file);
				FILE_FLAG = 0;
				break;
			}
		}

		// If reading is incomplete, write to file
		if (FILE_FLAG)
			write(file, buf_temp, strlen(buf_temp)+1);

	}
	if (byte_count > 0)
		printf("The file transfer is successful.\nSize of file = %d bytes\nNo. of words = %d\n", byte_count, word_count);

	// Close the socket
	close(sockfd);
}


