/*
TCP Client

Sayan Sinha
16CS10048
*/


#include <stdio.h> 
#include <fcntl.h> 
#include <errno.h> 
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <time.h>
#define PORT 8000
#define BUF_SIZE 6

int main()
{
  int     sockfd ;
  struct sockaddr_in  serv_addr;

  int i;
  char buf[100];

  /* Opening a socket is exactly similar to the server process */
  if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
    perror("Unable to create socket\n");
    exit(0);
  }

  // Set server address
  serv_addr.sin_family  = AF_INET;
  inet_aton("127.0.0.1", &serv_addr.sin_addr);
  serv_addr.sin_port  = htons(PORT);

  // Connect to server and handshake
  if ((connect(sockfd, (struct sockaddr *) &serv_addr,
            sizeof(serv_addr))) < 0) {
    perror("Unable to connect to server\n");
    exit(0);
  }
  else
    printf("Connection established\n");

  // Take input from user about filename
  // printf("Enter file name: ");
  // gets(buf);

  // // Send filename to server
  // send(sockfd, buf, strlen(buf) + 1, 0);
  // printf("Sent filename %s from client\n", buf);
  
  // // Delay ensures server gets enough time to send data to pipe
  // delay(100000);
  int word_count = 0;
  int read_word = 1;

  for (i = 1; ; i++)  // use i as a count variable
  {
    char buf_temp[BUF_SIZE+1];
    memset(buf_temp, 0, BUF_SIZE+1);

    buf_temp[BUF_SIZE] = '\0';

    printf("Receiving from server\n");
    int n = recv(sockfd, buf_temp, BUF_SIZE, 0);
    printf("%d\n", n);
    if (n > 0)
    {
      int i;
      // byte_count += n;
      for (i = 0; i < n; i++)
      {
        if (buf_temp[i] != '\n' && buf_temp[i] != '\0' && read_word)
        {
          word_count++;
          read_word = 0;
          continue;
        }
        if (buf_temp[i] == '\n')
          read_word = 1;
      }
    }
    else
      break;
  }
  // if (byte_count > 0)
  printf("The file transfer is successful.\n\nNo. of words = %d\n", word_count);

  // Close the socket
  close(sockfd);
}

