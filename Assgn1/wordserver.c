// A Server to send words in a file to a client.
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
    struct sockaddr_in servaddr; 
    struct sockaddr_in cliaddr;
    int n; 
    socklen_t len;
    char buffer[MAXLINE]; 
    int FILE_FLAG = 0;      // 0 indicates file is not open
                            // 1 indicates file is closed
      
    // Create socket file descriptor 
    sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if ( sockfd < 0 ) { 
        perror("socket creation failed"); 
        exit(EXIT_FAILURE); 
    } 
      
    memset(&servaddr, 0, sizeof(servaddr)); 
    FILE *fread;        // variable for file handling
    
    // set server port
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
    while(1) 
    {  

        len = sizeof(cliaddr);
        printf("Waiting for client.\n");
        n = recvfrom(sockfd, (char *)buffer, MAXLINE, 0, 
                (struct sockaddr *) &cliaddr, &len); 
        buffer[n] = '\0'; 
        printf("%s\n", buffer);

        if (!FILE_FLAG)
        {
            fread = fopen(buffer, "r");
            if (fread == NULL)
            {
                printf("Cannot open file \n");

                char *text = "NOTFOUND";

                sendto(sockfd, (const char *)text, strlen(text)+1, 0, 
            			(const struct sockaddr *) &cliaddr, sizeof(cliaddr)); 
                
            }
            else
                FILE_FLAG = 1;
        }
        if (FILE_FLAG)
        {
            char x[1024];
            fscanf(fread, " %1023s", x);
            sendto(sockfd, (const char *)x, strlen(x)+1, 0, 
                    (const struct sockaddr *) &cliaddr, sizeof(cliaddr)); 
            printf("Sent %s from server\n", x);

            // Close the file if END is encountered to faciliate an always on server
            if (strcmp(x, "END")==0)
            {
                fclose(fread);
                FILE_FLAG = 0;
            }
        }

    } 
      
    return 0; 
} 

