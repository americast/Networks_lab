// The Server Program
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
      
    // Create socket file descriptor 
    sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if ( sockfd < 0 ) { 
        perror("socket creation failed"); 
        exit(EXIT_FAILURE); 
    } 
      
    memset(&servaddr, 0, sizeof(servaddr)); 
    memset(&cliaddr, 0, sizeof(cliaddr)); 
      
    servaddr.sin_family    = AF_INET; 
    servaddr.sin_addr.s_addr = INADDR_ANY; 
    servaddr.sin_port = htons(8181); 
      
    // Bind the socket with the server address 
    if ( bind(sockfd, (const struct sockaddr *)&servaddr,  
            sizeof(servaddr)) < 0 ) 
    { 
        perror("bind failed"); 
        exit(EXIT_FAILURE); 
    } 
    
    printf("\nServer Running....\n");
  
    int n; 
    socklen_t len;
    char buffer[MAXLINE]; 
 
    // Receive the file name from client in buffer variable
    len = sizeof(cliaddr);
    n = recvfrom(sockfd, (char *)buffer, MAXLINE, 0, 
            ( struct sockaddr *) &cliaddr, &len); 
    buffer[n] = '\0'; 
    printf("Name of file : %s\n", buffer); 
      
    // Open the file requested by client

    char *errcode = "NOTFOUND";
    char word[MAXLINE];
    FILE * fp = fopen(buffer,"r");

    // If the file requested is not found in local directory, exit program
    if(!fp)
    {
            sendto(sockfd, (const char *)errcode, strlen(errcode), 0, (const struct sockaddr *) &cliaddr, sizeof(cliaddr));
            exit(EXIT_FAILURE);
    }
    // If file is found, send "HELLO" to client
    else
    {
        fscanf(fp,"%s",word);
        sendto(sockfd, (const char *)word, strlen(word), 0, (const struct sockaddr *) &cliaddr, sizeof(cliaddr));
            
    }
    
    //Continue sending WORDi upon request by client, until END is reached
    while(strcmp(word,"END"))
    {   
        len = sizeof(cliaddr);
        n = recvfrom(sockfd, (char *)buffer, MAXLINE, 0, ( struct sockaddr *) &cliaddr, &len);
        buffer[n] = '\0'; 
        printf("%s\n",buffer);
        fscanf(fp,"%s",word);
        sendto(sockfd, (const char *)word, strlen(word), 0, (const struct sockaddr *) &cliaddr, sizeof(cliaddr));
        
    }
    
    fclose(fp);
    close(sockfd); 
    return 0; 
}