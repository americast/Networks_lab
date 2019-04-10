#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <fcntl.h>
#include <errno.h>

#define PORT 5001

int main()
{
    int sockfd;
    struct sockaddr_in servaddr;
    
    int N = 1;
    
    if((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        perror("Unable to create socket");
        exit(EXIT_FAILURE);
    }
    
    servaddr.sin_family = AF_INET;
    inet_aton("127.0.0.1", &servaddr.sin_addr);
    servaddr.sin_port = htons(PORT);
    

    if (connect(sockfd, (struct sockaddr *) &servaddr, sizeof(servaddr)) < 0)
    {
        perror("Unable to connect to server");
        exit(EXIT_FAILURE);
    }
    
    if (fcntl(sockfd, F_SETFL, O_NONBLOCK) < 0)
    {
        perror("Unable to convert to non-blocking");
        exit(EXIT_FAILURE);
    }
    
    while(1)
    {
        int num = 1 + ((int)rand() % 3);
        sleep(num);
        if (N < 6)
        {
            char buf[100];
            int N_arr[1];
            N_arr[0] = N;
            strcpy(buf, "Message");
            memcpy(buf+8, N_arr, 4);
            send(sockfd, buf, 12, 0);
            printf("Message %d sent\n", N++);
        }
        
        char buf[100];
        int n = recv(sockfd, buf, 18, 0);
        if (n >= 0)
        {
            char IP[100];
            memcpy(IP, buf, 4);
            IP[4] = '\0';
            short port[1];
            memcpy(port, buf+4, 2);
            int port_here = ntohs(port[0]);
            char buf_here[100];
            memcpy(buf_here, buf+6, 8);
            int N_arr[1];
            memcpy(N_arr, buf+13, 4);
            printf("Client: Received %s from %s:%d\n", buf_here, IP, port_here);
            
        }
    }
    
    close(sockfd);
}
