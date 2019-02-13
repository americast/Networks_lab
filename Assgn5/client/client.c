/*
Client

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

#define B 20
#define PORT 20000

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
	serv_addr.sin_port	= htons(PORT);

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
	
	int file, FILE_FLAG = 1, byte_count = 0, word_count = 0;
	int read_word = 1;

	char code;
	int len;
	int n = recv(sockfd, &code, 1, MSG_WAITALL);
	if (n < 0)
	{
		perror("Error");
		exit(EXIT_FAILURE);
	}
	if (n==0)
	{
		printf("Connection broken\n");
		exit(EXIT_FAILURE);
	}

	if (code != 'L')		// i.e. is the code is 'E'
	{
		printf("File not found\n");
		exit(EXIT_SUCCESS);
	}

	// Open the file
	file = open(buf, O_CREAT | O_WRONLY | O_TRUNC, S_IRUSR | S_IWUSR);

	n = recv(sockfd, &len, sizeof(len), MSG_WAITALL);
	if (n < 0)
	{
		perror("Error");
		exit(EXIT_FAILURE);
	}
	if (n==0)
	{
		printf("Connection broken\n");
		exit(EXIT_FAILURE);
	}

	int iter, last_block_size;
	for (iter = 1; iter <= len/B + 1; iter++)	// use i as a count variable
	{
		int exp_len;
		if (iter == len/B + 1)
			exp_len = len % B;		// Checks if this is the last iter
		else
			exp_len = B;

		char buf_temp[exp_len+1];
		memset(buf_temp, 0, exp_len+1);

		n = recv(sockfd, buf_temp, exp_len, MSG_WAITALL);

		if (n < 0)
		{
			perror("Some error occured");
			close(sockfd);
			exit(EXIT_FAILURE);
		}
		else
		{
			byte_count += n;  // For parity checking at the end
			last_block_size = n;
		}

		printf("Received %s\n", buf_temp);
		// If reading is incomplete, write to file
		if (FILE_FLAG)
			write(file, buf_temp, exp_len);

	}
	if (byte_count == len)
	{
		printf("The file transfer is successful.\n");
		printf("Total number of blocks received: %d\n", iter - 1);
		printf("Last block size: %d\n", last_block_size);
	}
	else
		printf("File transfer unsucessful.\n");

	// Close the socket
	close(sockfd);
	close(file);
}


