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
    servaddr.sin_port = htons(8181); 
    if (r_bind(sockfd, (const struct sockaddr *)&servaddr,  sizeof(servaddr)) < 0)
    {
    	perror("Unable to bind");
    	exit(EXIT_FAILURE);
    }
    printf("Binding complete\n");
    printf("Server running...\n");
    while(1)
    {
    	char bufn[100];
    	int clilen = sizeof(cliaddr);
    	r_recvfrom(sockfd, bufn, 100, ( struct sockaddr *) &cliaddr, &clilen);
    	printf("Received: %s\n", bufn);
    }
}