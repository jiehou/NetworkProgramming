#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <errno.h>
#include <signal.h>
#include "message_object.h"

const int kMaxLine      = 4096;
const int kServPort     = 54321;

static int count;
static void sig_int(int signo) {
    printf("\n pingserver received %d datagrams\n", count);
    exit(0);
}

int main(int argc, char** argv) {
    if(argc != 2) error(1, 0, "usage: pingserver <sleeptime>");
    count = 0;
    int sleeptime = atoi(argv[1]);
    
    int listenfd = socket(AF_INET, SOCK_STREAM, 0);
    struct sockaddr_in servaddr;
    bzero(&servaddr, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    servaddr.sin_port = htons(kServPort);

    int ret1 = bind(listenfd, (struct sockaddr*)&servaddr, sizeof(servaddr));
    if(ret1 < 0) error(1, errno, "pingserver: bind failed");

    int ret2 = listen(listenfd, 1024);
    if(ret2 < 0) error(1, errno, "pingserver: listen failed");

    signal(SIGINT, sig_int);
    signal(SIGPIPE, SIG_IGN); // SIG_IGN: ignore

    int connfd;
    struct sockaddr_in cliaddr;
    socklen_t clilen = sizeof(cliaddr);
    connfd = accept(listenfd, (struct sockaddr*)&cliaddr, &clilen);
    if(connfd < 0) error(1, errno, "pingserver: bind failed");

    MessageObject message;
    for(;;) {
        int n = read(connfd, (char*)&message, sizeof(MessageObject));
        if(n < 0) error(1, errno, "pingserver: error read");
        else if(n == 0) error(1, 0, "pingserver: client closed");
        printf("pingserver: received %d bytes\n", n, message);
        count++;
        switch(ntohl(message.type)) {
            case MSG_TYPE1:
                printf("pingserver: process MSG_TYPE1\n");
                break;
            case MSG_TYPE2:
                printf("pingserver: process MSG_TYPE2\n");
                break;
            case MSG_PING: { //@NOTE: in this case dont forget 
                MessageObject pongmsg;
                pongmsg.type = MSG_PONG;
                sleep(sleeptime);
                ssize_t ret = send(connfd, (char*)&pongmsg, sizeof(pongmsg), 0);
                if(ret < 0) error(1, errno, "pingserver: send failure");
                break;
            }
            default:
                error(1, 0, "pingserver: unknown message type\n");
        }
    }
}