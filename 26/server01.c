#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <errno.h>
#include <signal.h>
#include <pthread.h>

const int kMaxLine      = 4096;
const int kServPort     = 54321;

void loop_echo(int fd) {
    char outbuf[kMaxLine];
    size_t outbuf_used = 0;
    ssize_t res;
    while (1) {
        char ch;
        res = recv(fd, &ch, 1, 0);
        if(res == 0) break;
        else if( res == -1) {
            error(1, errno, "read error");
            break;
        }
        if(outbuf_used < sizeof(outbuf)) outbuf[outbuf_used++] = ch;
        if(ch == '\n') {
            send(fd, outbuf, outbuf_used, 0);
            outbuf_used = 0;
            continue;
        }
    }
}

void* thread_run(void* arg) {
    pthread_detach(pthread_self());
    int fd = *((int*)arg);
    printf("[D] fd: %d\n", fd);
    loop_echo(fd);
    close(fd);
    return NULL;
}

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

int main(int argc, char** argv) {
    int listenfd = tcp_server_listen(kServPort);
    pthread_t tid;
    while (1)
    {
       struct  sockaddr_storage ss;
       socklen_t slen = sizeof(ss);
       int fd = accept(listenfd, (struct sockaddr*)&ss, &slen);
       if(fd < 0) error(1, errno, "accept failed");
       else pthread_create(&tid, NULL, &thread_run, &fd);
    }
    
}