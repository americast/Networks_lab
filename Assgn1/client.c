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
    
    char *filename = "test.txt"; 
      
    sendto(sockfd, (const char *)filename, strlen(filename)+1, 0, 
			(const struct sockaddr *) &servaddr, sizeof(servaddr)); 
    printf("Filename sent from client\n"); 
           
    int n; 
    socklen_t len;
    char buffer[MAXLINE]; 
    len = sizeof(cliaddr);
    n = recvfrom(sockfd, (char *)buffer, MAXLINE, 0, 
			( struct sockaddr *) &cliaddr, &len);     
    close(sockfd); 
    printf("%s\n", buffer);
    FILE *fout;
    fout = fopen("test_out.txt", "w");
    if (strcmp(buffer,"NOTFOUND"))
        printf("File not found\n");
    else
    {
        while(1)
        {
            char *word = "WORD"; 
              
            sendto(sockfd, (const char *)word, strlen(word)+1, 0, 
                    (const struct sockaddr *) &servaddr, sizeof(servaddr)); 


            n = recvfrom(sockfd, (char *)buffer, MAXLINE, 0, 
                        ( struct sockaddr *) &cliaddr, &len);
            if (strcmp(buffer, "END"))
                return 0;  
            else if (!strcmp(buffer,"HELLO"))
            {
                fprintf(fout, "%s ", buffer);
            }
        }
            
    }
    return 0; 
}
 

