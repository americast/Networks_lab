// The client program
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
    memset(&cliaddr, 0, sizeof(cliaddr)); 
      
    // Server information 
    servaddr.sin_family = AF_INET; 
    servaddr.sin_port = htons(8181); 
    servaddr.sin_addr.s_addr = INADDR_ANY; 
      
    int n;
    socklen_t len; 

    // Enter the file name from user
    char file[MAXLINE];
    printf("Enter the file name\n");
    scanf("%s",file);

    
    // Send the file name to Server
    sendto(sockfd, (const char *)file, strlen(file), 0, (const struct sockaddr *) &servaddr, sizeof(servaddr)); 
    printf("File name sent from client\n");   

    // buffer will store data received from server
    char buffer[MAXLINE]; 
    len = sizeof(servaddr);
    n = recvfrom(sockfd, (char *)buffer, MAXLINE, 0, ( struct sockaddr *) &servaddr, &len); 
    buffer[n] = '\0'; 

    //If the server does not find file with given name in local directory, then exit program
    if(!strcmp("NOTFOUND",buffer))
    {
      printf("File Not Found");
      exit(EXIT_FAILURE);
    }

    //If server finds the file in local directory
    if(!strcmp("HELLO",buffer))
    {

      //Create a file of name output.txt
      printf("Received \"HELLO\" from Server. Creating output.txt\n");
      FILE *fp = fopen("output.txt","w");
      int i = 0;

      while(1)
      { 
        i++;

        // word variable shall store WORDi to be retreived from server. Finding value of word:
        char word[MAXLINE];
        char temp[MAXLINE];
        sprintf(temp, "%d", i);
        strcpy(word,"WORD");
        strcat(word,temp);

        printf("Requesting %s from server\n",word);
        //Request the WORDi from server
        sendto(sockfd, (const char *)word, strlen(word), 0, (const struct sockaddr *) &servaddr, sizeof(servaddr));

        //WORDi received from server is stored in buffer
        len = sizeof(servaddr);
        n = recvfrom(sockfd, (char *)buffer, MAXLINE, 0, ( struct sockaddr *) &servaddr, &len); 
        buffer[n] = '\0'; 

        printf("Received \"%s\" from server\n",buffer);
        //If buffer == "END", then end of file is reached. client closes output.txt and exits
        if(!strcmp(buffer,"END"))
        {
          fclose(fp);
          printf("Closed file.\n");
          break;
        }

        //If end of file is not reached, write the word in output.txt
        else
        {
          fprintf(fp,"%s\n",buffer);
        }   
      }
    }

    close(sockfd); 
    return 0; 
}