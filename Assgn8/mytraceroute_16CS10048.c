/***********

Assignment 8
Sayan Sinha
16CS10048

***********/

#include <stdio.h>
#include <stdlib.h>
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
#include <time.h>

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

        strcpy(ip, IPbuffer);
    }

    printf("Target IP address: %s\n\n",(ip));
    int S1, S2;
    int exit_flag;
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
    saddr_raw.sin_port = htons(LISTEN_PORT);           // set to any port because port is only
                                                       // present in UDP layer, not IP layer
    saddr_raw.sin_addr.s_addr = inet_addr(LISTEN_IP);  // set it to hardware localhost
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

    char buf[1024];
    memset(buf, 0, 1024);   // Set to zero

    hdrip = ((struct iphdr * ) buf);
    hdrudp = ((struct udphdr * )(buf + iphdrlen));
    
    hdrip->ihl = 5;
    hdrip->version = 4;
    hdrip->protocol = 17;
    hdrip->id = htonl(0);
    hdrip->check = 0;
    hdrip->tos = 0;
    hdrip->frag_off = 0;
    hdrip->tot_len = htons(iphdrlen + udphdrlen + 52);      // Full length
    hdrip->saddr = saddr_raw.sin_addr.s_addr;
    hdrip->daddr = daddr_raw.sin_addr.s_addr;

    hdrudp->source = saddr_raw.sin_port;
    hdrudp->dest = daddr_raw.sin_port;
    hdrudp->len = htons(udphdrlen + 52);
    hdrudp->check = 0;
    // hdrudp->ui_sum = 0;

    // printf("buf: %s\n", buf);

    char buf2[2048];
    // int clilen = sizeof(raddr_raw);

    fd_set sock;
    struct timeval tv;
    int ttl_here = 1;
    int count = 0;
    while(1)
    {
        hdrip->ttl = ttl_here++;
        int s = sendto(S1, buf, iphdrlen + udphdrlen + 52, 0, (struct sockaddr *) &daddr_raw, sizeof(daddr_raw));
        clock_t before = clock();
        if (s < 0)
        {
            perror("Error in sending");
            exit(EXIT_FAILURE);
        }
        tv.tv_sec = 1;
        FD_ZERO(&sock);
        FD_SET(S2, &sock);
        int selected = select(S2 + 1, &sock, NULL, NULL, &tv);
        if (FD_ISSET(S2, &sock))
        {
            count = 0;
            memset(buf2, 0, 2048);  // Set recv buffer to zero
            int clilen = sizeof(raddr_raw);
            int r = recvfrom(S2, buf2, 2048, 0, (struct sockaddr *) &raddr_raw, &clilen);
            clock_t difference = clock() - before;
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
                hdricmp_here = ((struct icmphdr * ) (buf2 + sizeof(struct iphdr)));
                printf("Hop_Count(%d) \t %s \t %fs\n", ttl_here - 1, inet_ntoa( * ((struct in_addr * ) &hdrip_here->saddr)), difference * 1000.0 / CLOCKS_PER_SEC);
                if (hdricmp_here->type == 3)    // Reached destination
                {
                    if (hdrip_here->saddr == daddr_raw.sin_addr.s_addr)
                    {
                        printf("\nSuccess\n");
                        exit_flag = EXIT_SUCCESS;
                    }
                    else
                    {
                        printf("\nReceived incorrect IP\n");
                        exit_flag = EXIT_FAILURE;
                    }
                    break;
                }
                else if (hdricmp_here->type != 11)  // At some intermediate hop
                {
                    ttl_here--;
                    printf("Some other type\n");
                    printf("%d\n",hdricmp_here->type);
                }

            }
            else
            {
                ttl_here--;
                printf("Didn't reach yet perhaps\n");
                continue;
            }
        }
        else
        {   
            // Timeout
            if (count<3)
            {
                count++;
                ttl_here--;
                continue;
            }
            if (count>=3)
            {
                printf("Hop_Count(%d)           *             *\n", ttl_here - 1);
                count = 0;
            }
            continue;
        }
    }

    close(S1);
    close(S2);
    exit(exit_flag);    
}