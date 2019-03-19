#include "rsocket.h"

int main() { 
    int sockfd; 
    struct sockaddr_in servaddr; 
  
    // Creating socket file descriptor 
    sockfd = r_socket(AF_INET, SOCK_DGRAM, 0);
    if ( sockfd < 0 ) { 
        perror("socket creation failed"); 
        exit(EXIT_FAILURE); 
    } 
  
    memset(&servaddr, 0, sizeof(servaddr)); 
      
    // Server information 
    servaddr.sin_family = AF_INET; 
    servaddr.sin_port = htons(8181); 
    servaddr.sin_addr.s_addr = INADDR_ANY; 
      
    int n;
    socklen_t len; 
    char *hello = "CLIENT:HELLO"; 
      
    if (r_sendto(sockfd, (const char *)hello, strlen(hello)+1, 
            (const struct sockaddr *) &servaddr, sizeof(servaddr)) < 0)
    {
        perror("Error in sending");
        exit(EXIT_FAILURE);
    }
    printf("Hello message sent from client\n"); 
           
    r_close(sockfd); 
    return 0; 
} 
