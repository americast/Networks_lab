#include<stdio.h>

#include<stdlib.h>

#include <sys/socket.h>

#include <arpa/inet.h>

#include <netinet/in.h>

#include <linux/ip.h> /* for ipv4 header */

#include <linux/udp.h> /* for upd header */

#include <unistd.h>

#include <string.h>

#include <signal.h>

#include <sys/wait.h>

#define MSG_SIZE 2048
# define LISTEN_PORT 8081
# define LISTEN_IP "127.0.0.1"
int main() {
    int udpfd;
    struct sockaddr_in saddr_udp, cliaddr;
    char msg[MSG_SIZE];
    udpfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (udpfd < 0) {
        perror("udp socket");
        exit(__LINE__);
    }
    saddr_udp.sin_family = AF_INET;
    saddr_udp.sin_port = htons(LISTEN_PORT);
    saddr_udp.sin_addr.s_addr = INADDR_ANY;

    if ( bind(udpfd, (const struct sockaddr *)&saddr_udp,  
                sizeof(saddr_udp)) < 0 ) 
    { 
        perror("bind failed"); 
        exit(EXIT_FAILURE); 
    }

    int len = sizeof(cliaddr);
    int n = recvfrom(udpfd, (char *)msg, 2048, 0, 
                ( struct sockaddr *) &cliaddr, &len);

    msg[n] = '\0';
    printf("Received: %s\n", msg);
    return 0;
}