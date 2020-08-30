#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <errno.h>
#include <signal.h>
#include <sys/un.h> /* domain packets */

const int kBufferSize = 4096;
const int kMaxLine    = 4096*2;

int main(int argc, char** argv) {
    if(argc != 2) error(1, 0, "usage: stream_server <local_path>");
    int listenfd, connfd;
    socklen_t clilen;
    struct sockaddr_un cliaddr, servaddr;
    listenfd = socket(AF_LOCAL, SOCK_STREAM, 0);
    if(listenfd < 0) error(1, errno, "stream_server: create failed");
    
    char *local_path = argv[1];
    unlink(local_path);
    printf("[D] local_path: %s\n", local_path);
    bzero(&servaddr, sizeof(servaddr));
    servaddr.sun_family = AF_LOCAL;
    strcpy(servaddr.sun_path, local_path);

    if(bind(listenfd, (struct sockaddr*)&servaddr, sizeof(servaddr)) < 0)
        error(1, errno, "stream_server: bind failed");

    if(listen(listenfd, 1024) < 0) {
        error(1, errno, "stream_server: listen failed");
    }

    clilen = sizeof(cliaddr);
    connfd = accept(listenfd, (struct sockaddr*)&cliaddr, &clilen);
    if(connfd < 0) {
        printf("[D] connfd %d\n", connfd);
        error(1, errno, "stream_server: accept failed");
    }

    char buf[kBufferSize];
    for(;;) {
        bzero(buf, sizeof(buf));
        if(read(connfd, buf, kBufferSize) == 0) {
            printf("stream_server: client quit\n");
            break;
        }
        printf("stream_server receive: %s", buf);
        
        char send_line[kMaxLine];
        bzero(send_line, sizeof(send_line));
        sprintf(send_line, "Hi, %s", buf);
        int nbytes = sizeof(send_line);
        if(write(connfd, send_line, nbytes) != nbytes) 
            error(1, errno, "stream_server: write error");
    }
    close(listenfd);
    close(connfd);
} 