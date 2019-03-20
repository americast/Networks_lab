#include "rsocket.h"

int main()
{
	int sockfd;
	struct sockaddr_in servaddr, cliaddr; 
	// printf("Here\n");
	sockfd = r_socket(AF_INET, SOCK_MRP, 0);
	if (sockfd < 0)
	{
		perror("Socket creation failed");
		exit(EXIT_FAILURE);
	}
	printf("Socket created\n");

	memset(&servaddr, 0, sizeof(servaddr)); 
	memset(&cliaddr, 0, sizeof(cliaddr)); 

	servaddr.sin_family    = AF_INET; 
    servaddr.sin_addr.s_addr = INADDR_ANY; 
    servaddr.sin_port = htons(50000 + (2 * 10048) + 1); 
    if (r_bind(sockfd, (const struct sockaddr *)&servaddr,  sizeof(servaddr)) < 0)
    {
    	perror("Unable to bind");
    	exit(EXIT_FAILURE);
    }
    printf("Binding complete\n");
    printf("Server running...\n");
    // while(1)
    // {
	char bufn[100];
	int clilen = sizeof(cliaddr);
	if (r_recvfrom(sockfd, bufn, 100, 0, ( struct sockaddr *) &cliaddr, &clilen) < 0)
	{
		printf("Receive error\n");
	}
	printf("Received: %s\n", bufn);
    // }

    // int clilen = sizeof(cliaddr);
    char bufn2[] = "B";
    r_sendto(sockfd, bufn2, strlen(bufn2) + 1, 0, ( struct sockaddr *) &cliaddr, sizeof(cliaddr));
    r_close(sockfd);
}