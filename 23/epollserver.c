#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <errno.h>
#include <signal.h>
#include <poll.h>
#include <fcntl.h>
#include <sys/epoll.h>

#define kMaxLine  4096

const int kMaxEvents     = 128;
const int kServPort     = 54321;

int tcp_nonblocking_server_listen(int port) {
    int listenfd = socket(AF_INET, SOCK_STREAM, 0);
    fcntl(listenfd, F_SETFL, O_NONBLOCK);

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

/**
 * epoll api:
 * epoll_create
 * epoll_ctl
 * epoll_wait
*/
int main(int argc, char** argv)  {
    int listenfd, sockfd;
    listenfd = tcp_nonblocking_server_listen(kServPort);

    int epfd = epoll_create1(0);
    if(epfd == -1) error(1, errno, "epoll create failed");
    struct epoll_event evt;
    evt.data.fd = listenfd;
    evt.events = EPOLLIN | EPOLLET;
    if(epoll_ctl(epfd, EPOLL_CTL_ADD, listenfd, &evt) == -1) error(1, errno, "epoll create failed");
    
    /* buffer where events are returned */
    struct epoll_event* evlist;
    evlist = calloc(kMaxEvents, sizeof(evt));
    int n, i;
    while (1) {
        n = epoll_wait(epfd, evlist, kMaxEvents, -1);
        printf("[I] epoll_wait wakeup\n");
        for(i = 0; i < n; ++i) {
            if((evlist[i].events & EPOLLERR) ||
               (evlist[i].events & EPOLLHUP) ||
               (!(evlist[i].events & EPOLLIN))) {
                   fprintf(stderr, "epoll error\n");
                   close(evlist[i].data.fd);
                   continue;
            }
            else if(listenfd == evlist[i].data.fd) {
                struct sockaddr_storage ss;
                socklen_t sslen = sizeof(ss);
                int fd = accept(listenfd, (struct sockaddr*)&ss, &sslen);
                if(fd == -1) error(1, errno, "accept failed");
                else {
                    fcntl(fd, F_SETFL, O_NONBLOCK); // make connection fd nonblock
                    evt.data.fd = fd;
                    evt.events = EPOLLIN | EPOLLET;
                    if(epoll_ctl(epfd, EPOLL_CTL_ADD, fd, &evt) == -1)
                        error(1, errno, "epoll_ctl: add connection fd failed");
                }
                continue;
            }
            else {
                sockfd = evlist[i].data.fd;
                printf("[I] get event on socket fd: %d\n", sockfd);
                while (1) {
                    char buf[1024];
                    n = read(sockfd, buf, sizeof(buf));
                    if(n < 0) {
                        if(errno != EAGAIN) {
                            error(1, errno, "read error");
                            close(sockfd);
                        }
                        break;
                    }
                    else if(n == 0) {
                        close(sockfd);
                        break;
                    }
                    else {
                        printf("[I] recv data: %s\n", buf);
                        if(write(sockfd, buf, n)  < 0) error(1, errno, "write error");
                    }
                }// inner while 
            }
        }// loop over events
    }// outer while
    free(evlist);
    close(listenfd);
    close(epfd);
    return 0;
}