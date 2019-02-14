/*
BoW Client

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
#define PORT_TCP 8002
#define BUF_SIZE 100

int main()
{
  int     sockfd ;  // Define sockets
  struct sockaddr_in  serv_addr;

  int i;
  char buf[100];

  if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {  // Open TCP socket
    perror("Unable to create socket\n");
    exit(EXIT_FAILURE);
  }

  // Set server address
  serv_addr.sin_family  = AF_INET;
  inet_aton("127.0.0.1", &serv_addr.sin_addr);
  serv_addr.sin_port  = htons(PORT_TCP);

  // Connect to server and handshake
  if ((connect(sockfd, (struct sockaddr *) &serv_addr,
            sizeof(serv_addr))) < 0) {
    perror("Unable to connect to server\n");
    exit(0);
  }
  else
    printf("Connection established\n");

  char req[] = "handshake";
  send(sockfd, req, strlen(req) + 1, 0);

  int word_count = 0, read_word = 1, null_count = 0, done_flag = 0;
  for (i = 1; ; i++)  // use i as a count variable
  {
    char buf_temp[BUF_SIZE+1];
    memset(buf_temp, 0, BUF_SIZE+1);

    buf_temp[BUF_SIZE] = '\0';  // Keep the last position null

    printf("Receiving from server\n");
    int n = recv(sockfd, buf_temp, BUF_SIZE, 0);  // Receive from server
    printf("No of bytes received: %d\n", n);
    int i;
    for (i = 0; i < n; i++)  // Number of words counter
    {
      if (buf_temp[i] == '\0')
        null_count++;
      if (buf_temp[i] != '\0' && null_count > 0)
        null_count = 0;
      if (null_count >= 2)
      {
        done_flag = 1;
        break;
      }
      if (i < n - 1  && buf_temp[i] == '\0' && buf_temp[i+1] == '\0')
      {
        done_flag = 1;
        break;
      }
      if (buf_temp[i] != '\n' && buf_temp[i] != '\0' && read_word)
      {
        word_count++;
        read_word = 0;
        continue;
      }
      if (buf_temp[i] == '\n' || buf_temp[i] == '\0')
        read_word = 1;
    }
  if (done_flag)
    break;
  }
  printf("\nNo. of words = %d\n", word_count);

  // Close the socket
  close(sockfd);
}

