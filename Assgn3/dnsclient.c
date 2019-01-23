
/*    THE CLIENT PROCESS */


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

main()
{
	int			sockfd ;
	struct sockaddr_in	serv_addr;

	int i;
	char buf[100];
	// for(i=0; i < 100; i++) buf[i] = '\0';

	printf("Enter hostname: ");
	gets(buf);

	/* Opening a socket is exactly similar to the server process */
	if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
		perror("Unable to create socket\n");
		exit(0);
	}

	/* Recall that we specified INADDR_ANY when we specified the server
	   address in the server. Since the client can run on a different
	   machine, we must specify the IP address of the server. 

	   In this program, we assume that the server is running on the
	   same machine as the client. 127.0.0.1 is a special address
	   for "localhost" (this machine)
	   
	/* IF YOUR SERVER RUNS ON SOME OTHER MACHINE, YOU MUST CHANGE 
           THE IP ADDRESS SPECIFIED BELOW TO THE IP ADDRESS OF THE 
           MACHINE WHERE YOU ARE RUNNING THE SERVER. 
    	*/

	serv_addr.sin_family	= AF_INET;
	inet_aton("127.0.0.1", &serv_addr.sin_addr);
	serv_addr.sin_port	= htons(8000);

	/* With the information specified in serv_addr, the connect()
	   system call establishes a connection with the server process.
	*/
	/* After connection, the client can send or receive messages.
	   However, please note that recv() will block when the
	   server is not sending and vice versa. Similarly send() will
	   block when the server is not receiving and vice versa. For
	   non-blocking modes, refer to the online man pages.
	*/
	// recv(sockfd, buf, 100, 0);
	// printf("%s\n", buf);

	
	// strcpy(buf,"Message from client");
	sendto(sockfd, (const char *)buf, strlen(buf)+1, 0, (const struct sockaddr *) &serv_addr, sizeof(serv_addr)); 
	
	printf("Waiting for server\n");
	char buf2[100];
	for (i = 0; i<100; i++)
		buf2[i] = '\0';
	int len = sizeof(serv_addr);
	int n = recvfrom(sockfd, buf2, 100, 0, ( struct sockaddr *) &serv_addr, &len);
	printf("IPs recieved: %s\n", buf2);

	close(sockfd);
}

