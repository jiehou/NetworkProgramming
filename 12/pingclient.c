#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <errno.h>
#include <signal.h>
#include "message_object.h"

const int kMaxLine              = 4096;
const int kServPort             = 54321;
const int kKeepAliveTime        = 10;
const int kKeepAliveInterval    = 3;
const int kKeepAliveProbetimes  = 3;

int main(int argc, char** argv) {
    if(argc != 2) error(1, 0, "usage: pingclient <ip_address>");
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    
    struct sockaddr_in servaddr;
    bzero(&servaddr, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(kServPort);
    inet_pton(AF_INET, argv[1], &servaddr.sin_addr);

    socklen_t servlen = sizeof(servaddr);
    int conn_ret = connect(sockfd, (struct sockadd*)&servaddr, servlen);
    if(conn_ret < 0) error(1, errno, "pingclient: connect failed");

    char recv_line[kMaxLine];
    int n;
    fd_set readmask;
    fd_set allreads;

    struct timeval tv;
    int heartbeats = 0;
    tv.tv_sec = kKeepAliveTime;
    tv.tv_usec = 0;

    MessageObject mobject;
    FD_ZERO(&allreads);
    FD_SET(sockfd, &allreads);
    for(;;) {
        readmask = allreads;
        int rc = select(sockfd+1, &readmask, NULL, NULL, &tv);
        if(rc < 0) error(1, errno, "pingclient: select failed");
        if(rc == 0) {
            if(++heartbeats > kKeepAliveProbetimes) error(1, 0, "pingclient: connection dead");
            printf("pingclient: sending heartbeat #%d\n", heartbeats);
            mobject.type = htonl(MSG_PING);
            rc = send(sockfd, (char*)&mobject, sizeof(mobject), 0);
            if(rc < 0) error(1, errno, "pingclient: send failure");
            tv.tv_sec = kKeepAliveInterval;
            continue;
        }
        if(FD_ISSET(sockfd, &readmask)) {
            n = read(sockfd, recv_line, kMaxLine);
            if(n < 0) error(1, errno, "pingclient: read error");
            else if(n == 0) error(1, 0, "pingclient: server terminated\n");
            printf("pingclient: received heartbeat, make heartbeats to 0\n");
            heartbeats = 0;
            tv.tv_sec = kKeepAliveTime;
        }
    }
}