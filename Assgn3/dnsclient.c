/*

DNS Client

16CS10048
Sayan Sinha

*/


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#define PORT_UDP 8000

main()
{
	int	sockfd ;	// Define socket 
	struct sockaddr_in	serv_addr;  // Server address

	int i;
	char buf[100];

	printf("Enter hostname: ");
	gets(buf);

	if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) { // Opening UDP socket
		perror("Unable to create socket\n");
		exit(0);
	}

	serv_addr.sin_family	= AF_INET;
	inet_aton("127.0.0.1", &serv_addr.sin_addr);
	serv_addr.sin_port	= htons(PORT_UDP);

	sendto(sockfd, (const char *)buf, strlen(buf)+1, 0, (const struct sockaddr *) &serv_addr, sizeof(serv_addr)); // Send hostname to server
	
	printf("Waiting for server\n");

	char buf2[100];
	for (i = 0; i<100; i++)  // Fill with zeros
		buf2[i] = '\0';
	int len = sizeof(serv_addr);
	while(1)
	{
		int n = recvfrom(sockfd, buf2, 100, 0, ( struct sockaddr *) &serv_addr, &len); // Receive IPs from server
		if (n==0)
			break;
		printf("IP recieved: %s\n", buf2);
	}

	close(sockfd);  // Close connection
}

