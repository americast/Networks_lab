// A Simple Client Implementation
#include <stdio.h> 
#include <stdlib.h> 
#include <unistd.h> 
#include <string.h> 
#include <sys/types.h> 
#include <sys/socket.h> 
#include <arpa/inet.h> 
#include <netinet/in.h> 
  
#define MAXLINE 1024 

int main() { 
    int sockfd; 
    struct sockaddr_in servaddr, cliaddr; 
  
    // Creating socket file descriptor 
    sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if ( sockfd < 0 ) { 
        perror("socket creation failed"); 
        exit(EXIT_FAILURE); 
    } 
  
    memset(&servaddr, 0, sizeof(servaddr)); 
      
    // Server information 
    servaddr.sin_family = AF_INET; 
    servaddr.sin_port = htons(8181); 
    servaddr.sin_addr.s_addr = INADDR_ANY; 
    
    char *hello = "CLIENT:HELLO"; 
      
    sendto(sockfd, (const char *)hello, strlen(hello), 0, 
            (const struct sockaddr *) &servaddr, sizeof(servaddr)); 
    printf("Hello message sent from client\n"); 
           
    // sendto(sockfd, (const char *)hello, strlen(hello), 0, 
    //         (const struct sockaddr *) &servaddr, sizeof(servaddr)); 
    // printf("Hello message sent from client\n");    


    int n; 
    socklen_t len;
    char buffer[MAXLINE], buffer2[MAXLINE]; 
    len = sizeof(cliaddr);
    n = recvfrom(sockfd, (char *)buffer, MAXLINE, 0, 
            ( struct sockaddr *) &cliaddr, &len);     
    close(sockfd); 
    printf("Rcv1: %s\n", buffer);

    
    // memset(&servaddr, 0, sizeof(servaddr)); 
    sendto(sockfd, (const char *)hello, strlen(hello), 0, 
            (const struct sockaddr *) &servaddr, sizeof(servaddr)); 
    printf("Hello message sent from client\n");
    memset(&cliaddr, 0, sizeof(cliaddr)); 
    n = recvfrom(sockfd, (char *)buffer, MAXLINE, 0, 
            ( struct sockaddr *) &cliaddr, &len);     
    close(sockfd); 
    printf("Rcv2: %s\n", buffer2);
    return 0; 
} 

