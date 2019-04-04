#include<stdio.h>
#include<stdlib.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <linux/ip.h> /* for ipv4 header */
#include <linux/udp.h> /* for upd header */
#include <linux/icmp.h> 
#include <unistd.h>
#include <string.h>
#include <signal.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <netdb.h>

#define MSG_SIZE 2048
# define LISTEN_PORT 8080
// #define LISTEN_IP "127.0.0.1"
# define LISTEN_IP "0.0.0.0"
int main(int argc, char *argv[]) {
    char domain[100];
    strcpy(domain, argv[1]);

    struct hostent *lh = gethostbyname(domain);

    char *IPbuffer; 

    int i;

    char all[] = "";

    char ip[100];

    // printf("Length: %d\n", lh->h_length);
    for (i = 0; i < lh->h_length; i++)
    {

        if (lh->h_addr_list[i] == NULL)
            break;
        IPbuffer = inet_ntoa(*((struct in_addr*) 
                               lh->h_addr_list[i])); 
        // printf("Here3\n");

        printf("IP buffer %d: %s\n", i+1, (IPbuffer));
        strcpy(ip, IPbuffer);
    }

    int S1, S2;
    struct sockaddr_in saddr_raw, daddr_raw, raddr_raw;
    memset(&saddr_raw, 0, sizeof(saddr_raw));
    memset(&daddr_raw, 0, sizeof(daddr_raw));
    memset(&raddr_raw, 0, sizeof(raddr_raw));

    struct iphdr *hdrip;
    struct udphdr *hdrudp;
    int iphdrlen = sizeof(struct iphdr);
    int udphdrlen = sizeof(struct udphdr);

    S1 = socket(AF_INET, SOCK_RAW, IPPROTO_UDP);
    if (S1 < 0)
    {
        perror("Could not create socket S1");
        exit(EXIT_FAILURE);
    }

    S2 = socket(AF_INET, SOCK_RAW, IPPROTO_ICMP);
    if (S2 < 0)
    {
        perror("Could not create socket S2");
        exit(EXIT_FAILURE);
    }

    saddr_raw.sin_family = AF_INET;
    saddr_raw.sin_port = htons(LISTEN_PORT);
    saddr_raw.sin_addr.s_addr = inet_addr(LISTEN_IP);
    int saddr_raw_len = sizeof(saddr_raw);


    daddr_raw.sin_family = AF_INET;
    daddr_raw.sin_port = htons(32164);
    daddr_raw.sin_addr.s_addr = inet_addr(ip);
    int daddr_raw_len = sizeof(daddr_raw);

    if (bind(S1, (struct sockaddr * ) &saddr_raw, saddr_raw_len) < 0) {
        perror("S1 bind error");
        exit(EXIT_FAILURE);
    }
    if (bind(S2, (struct sockaddr * ) & saddr_raw, saddr_raw_len) < 0) {
        perror("S2 bind error");
        exit(EXIT_FAILURE);
    }


    if (setsockopt(S1, IPPROTO_IP, IP_HDRINCL, &(int){1}, sizeof(int)) < 0)
    {
            perror("Could not set socket options 1");
            exit(EXIT_FAILURE);
    }

    char buf[1000];
    memset(buf, 0, 100);

    hdrip = ((struct iphdr * ) buf);
    hdrudp = ((struct udphdr * )(buf + iphdrlen));
    
    hdrip->ihl = 5;
    hdrip->version = 4;
    hdrip->ttl = 1;
    hdrip->protocol = 17;
    hdrip->id = htonl(0);
    hdrip->check = 0;
    hdrip->tos = 0;
    hdrip->frag_off = 0;
    hdrip->tot_len = htons(iphdrlen + udphdrlen + 52);
    hdrip->saddr = saddr_raw.sin_addr.s_addr;
    hdrip->daddr = daddr_raw.sin_addr.s_addr;

    hdrudp->source = saddr_raw.sin_port;
    hdrudp->dest = daddr_raw.sin_port;
    hdrudp->len = htons(udphdrlen + 52);
    // hdrudp->ui_sum = 0;

    // printf("buf: %s\n", buf);
    int s = sendto(S1, buf, iphdrlen + udphdrlen + 52, 0, (struct sockaddr *) &daddr_raw, sizeof(daddr_raw));
    if (s < 0)
    {
        perror("Error in sending");
        exit(EXIT_FAILURE);
    }

    char buf2[2048];
    int clilen = sizeof(raddr_raw);

    fd_set sock;
    struct timeval tv;
    while(1)
    {
        tv.tv_sec = 1;
        FD_ZERO(&sock);
        FD_SET(S2, &sock);
        int selected = select(S2 + 1, &sock, NULL, NULL, &tv);
        if (FD_ISSET(S2, &sock))
        {
            memset(buf2, 0, 2048);
            int r = recvfrom(S2, buf2, 2048, 0, (struct sockaddr *) &raddr_raw, &clilen);
            if (r < 0)
            {
                perror("Error in receive");
                exit(EXIT_FAILURE);
            }
            struct iphdr *hdrip_here;
            struct icmphdr *hdricmp_here;
            hdrip_here = ((struct iphdr * ) buf2);
            if (hdrip_here->protocol == 1)
            {
                hdricmp_here = ((struct icmphdr * ) buf2 + sizeof(struct iphdr));
                if (hdricmp_here->type == 3)
                {
                    if (hdrip_here->saddr == daddr_raw.sin_addr.s_addr)
                    {
                        printf("Done\n");
                        exit(EXIT_SUCCESS);
                    }
                    else
                    {
                        printf("Received incorrect IP\n");
                    }
                }
                else
                {
                    printf("Some other type\n");
                    printf("%u\n",hdricmp_here->type);
                    printf("IP is: %s\n", inet_ntoa( * ((struct in_addr * ) &hdrip_here->saddr)));
                }

            }
            else
            {
                printf("Didn't reach yet perhaps\n");
            }
        }
        else
        {   
            printf("Timeout\n");
            continue;
        }
    }

    close(S1);
    close(S2);

    /*
    exit(0);


    
    int rawfd, udpfd;
    struct sockaddr_in saddr_raw, saddr_udp;
    struct sockaddr_in raddr;
    int saddr_raw_len, saddr_udp_len;
    int raddr_len;
    char msg[MSG_SIZE];
    int msglen;
    pid_t pid = fork();
    if (pid == 0) {
        struct iphdr hdrip;
        struct udphdr hdrudp;
        int iphdrlen = sizeof(hdrip);
        int udphdrlen = sizeof(hdrudp);



        rawfd = socket(AF_INET, SOCK_RAW, IPPROTO_UDP);
        if (rawfd < 0) {
            perror("raw socket");
            exit(__LINE__);
        }
        saddr_raw.sin_family = AF_INET;
        saddr_raw.sin_port = htons(LISTEN_PORT);
        saddr_raw.sin_addr.s_addr = INADDR_ANY; //inet_addr(LISTEN_IP);
        saddr_raw_len = sizeof(saddr_raw);
        if (bind(rawfd, (struct sockaddr * ) & saddr_raw, saddr_raw_len) < 0) {
            perror("raw bind");
            exit(__LINE__);
        }

        while (1) {
            raddr_len = sizeof(raddr);
            msglen = recvfrom(rawfd, msg, MSG_SIZE, 0, (struct sockaddr * ) &
                raddr, & raddr_len);
            if (msglen <= 0) //ignoring all the errors is not a good idea.
                continue;
            hdrip = * ((struct iphdr * ) msg);
            hdrudp = * ((struct udphdr * )(msg + iphdrlen));
            if (hdrudp.dest != saddr_raw.sin_port)
                continue;
            msg[msglen] = 0;

            printf("RAW socket: ");
            printf("hl: %d, version: %d, ttl: %d, protocol: %d",
                hdrip.ihl, hdrip.version, hdrip.ttl, hdrip.protocol);
            printf(", src: %s", inet_ntoa( * ((struct in_addr * ) &
                hdrip.saddr)));
            printf(", dst: %s", inet_ntoa( * ((struct in_addr * ) &
                hdrip.daddr)));
            printf("\nRAW socket: \tUdp sport: %d, dport: %d",
                ntohs(hdrudp.source), ntohs(hdrudp.dest));
            printf("\nRAW socket: \tfrom: %s:%d",
                inet_ntoa(raddr.sin_addr), ntohs(raddr.sin_port));
            printf("\nRaw Socket: \tUDP payload: %s",
                msg + iphdrlen + udphdrlen);
            printf("\n");
        }
        close(rawfd);
    } else {
        udpfd = socket(AF_INET, SOCK_DGRAM, 0);
        if (udpfd < 0) {
            perror("udp socket");
            exit(__LINE__);
        }
        saddr_udp.sin_family = AF_INET;
        saddr_udp.sin_port = htons(LISTEN_PORT);
        saddr_udp.sin_addr.s_addr = INADDR_ANY; //inet_addr(LISTEN_IP);
        saddr_udp_len = sizeof(saddr_udp);
        if (bind(udpfd, (struct sockaddr * ) & saddr_udp, saddr_udp_len) < 0) {
            perror("raw bind");
            exit(__LINE__);
        }
        while (1) {
            raddr_len = sizeof(raddr);
            msglen = recvfrom(udpfd, msg, MSG_SIZE, 0, (struct sockaddr * ) &
                raddr, & raddr_len);
            msg[msglen] = 0;
            printf("UDP: recv len: %d, recvfrom: %s:%d\n", msglen,
                inet_ntoa(raddr.sin_addr), ntohs(raddr.sin_port));
            printf("UDP: payload: %s\n", msg);
        }
        close(udpfd);
    }
    return 0;
    */
    
}