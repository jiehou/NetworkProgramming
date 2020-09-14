#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <errno.h>
#include <signal.h>
#include <poll.h>

const int kInitSize     = 128;
const int kMaxLine      = 4096;
const int kServPort     = 54321;

int tcp_server_listen(int port) {
    int listenfd = socket(AF_INET, SOCK_STREAM, 0);

    struct sockaddr_in servaddr;
    bzero(&servaddr, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    servaddr.sin_port = htons(kServPort);

    int on = 1;
    setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on));

    int ret1 = bind(listenfd, (struct sockaddr*)&servaddr, sizeof(servaddr));
    if(ret1 < 0) error(1, errno, "bind failed");

    int ret2 = listen(listenfd, 1024);
    if(ret2 < 0) error(1, errno, "listen failed");
    return listenfd;
}

/* tcp server utilizing multiplexing I/O function poll */
int main(void) {
    int listenfd, connfd;
    int ready_num;
    ssize_t n;
    char buf[kMaxLine];
    
    listenfd = tcp_server_listen(kServPort);
    /* initialize pollfd array*/
    struct pollfd fds[kInitSize];
    fds[0].fd = listenfd;
    fds[0].events = POLLRDNORM;
    /* -1: not initialized */
    for(int i = 1; i < kInitSize; ++i) fds[i].fd = -1;
    struct sockaddr_in cliaddr;
    for(;;) {
        ready_num = poll(fds, kInitSize, -1);
        if(ready_num <  0) error(1, errno, "poll failed");

        if(fds[0].revents & POLLRDNORM) {
            socklen_t clilen = sizeof(cliaddr);
            connfd = accept(listenfd, (struct sockaddr*)&cliaddr, &clilen);
            int i;
            for(i = 1; i < kInitSize; ++i) {
                if(fds[i].fd < 0) {
                    fds[i].fd = connfd;
                    fds[i].events = POLLRDNORM;
                    break;
                }
            }
            if(i == kInitSize) error(1, errno, "cannot hold more than 128 clients");
            if(--ready_num <= 0) continue;
        }

        for(int i = 1; i < kInitSize; ++i) {
            int sockfd = fds[i].fd;
            if(sockfd < 0) continue;
            if(fds[i].revents & (POLLRDNORM | POLLERR)) {
                n = read(sockfd, buf, kMaxLine);
                if(n > 0) {
                    if(write(sockfd, buf, n) < 0) error(1, errno, "write failed");
                }
                else if(n == 0 || errno == ECONNRESET) {
                    close(sockfd);
                    fds[i].fd = -1;
                }
                else error(1, errno, "read failed");
                if(--ready_num <= 0) break;
            }
        }
    }
}