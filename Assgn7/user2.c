#include "rsocket.h"

int main() { 
    int sockfd; 
    struct sockaddr_in servaddr, ownconfig; 
  
    // Creating socket file descriptor 
    sockfd = r_socket(AF_INET, SOCK_DGRAM, 0);
    if ( sockfd < 0 ) { 
        perror("socket creation failed"); 
        exit(EXIT_FAILURE); 
    } 
  
    memset(&servaddr, 0, sizeof(servaddr)); 
    memset(&ownconfig, 0, sizeof(ownconfig)); 
      
    // Server information 
    servaddr.sin_family = AF_INET; 
    servaddr.sin_port = htons(50000 + (2 * 10048) + 1); 
    servaddr.sin_addr.s_addr = INADDR_ANY; 



    ownconfig.sin_family    = AF_INET; 
    ownconfig.sin_addr.s_addr = INADDR_ANY; 
    ownconfig.sin_port = htons(50000 + (2 * 10048)); 
    if (r_bind(sockfd, (const struct sockaddr *)&ownconfig,  sizeof(ownconfig)) < 0)
    {
        perror("Unable to bind");
        exit(EXIT_FAILURE);
    }
    printf("Binding complete\n");
      
    int n;
    socklen_t len;
    char buf[] = "A quick brown fox jumps over the lazy dog.";
    // char *hello = "CLIENT:HELLO"; 
    int i;
    for (i = 0; i < strlen(buf); i++)
    {
        char buf_here[2];
        sprintf(buf_here, "%c", buf[i]);
        buf_here[1] = '\0';
        if (r_sendto(sockfd, (const char *)buf_here, 2, 
                (const struct sockaddr *) &servaddr, sizeof(servaddr)) < 0)
        {
            perror("Error in sending");
            exit(EXIT_FAILURE);
        }
        printf("%s message sent from client\n", buf_here); 
    }
           
    r_close(sockfd); 
    return 0; 
} 
