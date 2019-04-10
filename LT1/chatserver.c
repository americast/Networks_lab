#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <fcntl.h>
#include <errno.h>

#define PORT 5001

struct client_details
{
  char IP[100];
  int port;
};

int max(int a, int b, int c, int d, int e, int f)
{
    if (a >= b && a >= c && a >= d && a >= e && a >= f)
     return a;
    if (b >= a && b >= c && b >= d && b >= e && b >= f)
     return b;
    if (c >= a && c >= b && c >= d && c >= e && c >= f)
     return c;
    if (d >= a && d >= b && d >= c && d >= e && d >= f)
     return d;
    if (e >= a && e >= b && e >= c && e >= d && e >= f)
     return e;     
    if (f >= a && f >= b && f >= c && f >= d && f >= e)
     return a;
}

int main()
{
    int sockfd, clientsockfd[] = {-1, -1, -1, -1, -1}, numclient;
    struct client_details clientId[5];
    struct sockaddr_in servaddr, cliaddr;
    
    int i;
    
    if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        perror("Unable to create socket");
        exit(EXIT_FAILURE);
    }
    
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = INADDR_ANY;
    servaddr.sin_port = htons(PORT);
    
    if (bind(sockfd, (struct sockaddr *) &servaddr, sizeof(servaddr)) < 0)
    {
        perror("Unable to bind to port");
        exit(EXIT_FAILURE);
    }
    
        
    if (fcntl(sockfd, F_SETFL, O_NONBLOCK) < 0)
    {
        perror("Unable to convert to non-blocking");
        exit(EXIT_FAILURE);
    }
    
    listen(sockfd, 5);
    
    fd_set sock;
    
    while(1)
    {
        FD_ZERO(&sock);
        FD_SET(sockfd, &sock);
        for (i = 0; i < numclient; i++)
            FD_SET(clientsockfd[i], &sock);
        int selected = select(max(sockfd, clientsockfd[0], clientsockfd[1], clientsockfd[2], clientsockfd[3], clientsockfd[4]) + 1, &sock, NULL, NULL, NULL);
        printf("Selected\n");
        if (FD_ISSET(sockfd, &sock))
        {
            printf("Waiting for new connection\n");
            int clilen = sizeof(cliaddr);
            int newsockfd = accept(sockfd, (struct sockaddr *) &cliaddr, &clilen);
            
            
            {
                clientsockfd[numclient] = newsockfd;
                strcpy(clientId[numclient].IP, inet_ntoa(cliaddr.sin_addr));
                clientId[numclient].port = ntohs(cliaddr.sin_port);
                printf("Server: Received a new connection from client %s:%d\n", clientId[numclient].IP, clientId[numclient].port);
                numclient++;
                printf("numclient: %d\n", numclient);
            }
            
        }
        else
        {
                printf("Waiting for msg\n");
                char buf[100];
                for (i = 0; i < numclient; i++)
                {
                    if (!FD_ISSET(clientsockfd[i], &sock))
                        continue;
                    int n = recv(clientsockfd[i], buf, 12, 0);
                    if (n < 0)
                        continue;
                    int N_arr[1];
                    memcpy(N_arr, buf + 8, 4);
                    printf("Server: Received %s %d from client %s:%d\n", buf, N_arr[0], clientId[i].IP, clientId[i].port);
                    if (numclient <= 1)
                    {
                        printf("Server: Insufficient clients, %s %d from client %s:%d dropped\n", buf, N_arr[0], clientId[i].IP, clientId[i].port);
                        continue;
                    }
                    
                    int j;
                    for (j = 0; j < numclient; j++)
                    {
                        if (i == j)
                            continue;
                        char buf_here[100];
                        char IP_here[100];
                        inet_aton(clientId[j].IP, IP_here);
                        memcpy(buf_here, IP_here, 4);
                        memcpy(buf_here+4, htons(clientId[j].port), 2);
                        memcpy(buf_here+6, buf, 12);
                        send(clientsockfd[j], buf_here, 18, 0);
                        printf("Server: Sent message %s %d from client %s:%d\n", buf, N_arr[0], clientId[j].IP, clientId[j].port);
                        
                    }
                }
        }
    }
    close(sockfd);
}
