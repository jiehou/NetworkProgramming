#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <errno.h>
#include <signal.h>

const int kMaxLine      = 4096;
const int kServPort     = 54321;

static int count;

static void sig_int(int signo) {
    printf("sig_int: received %d datagrams\n", count);
    exit(0);
}

static void sig_pipe(int signo) {
    printf("sig_pipe: received %d datagrams\n", count);
    exit(0);
}

int main(int argc, char** argv) {
    count = 0;
    
    int listenfd;
    listenfd = socket(AF_INET, SOCK_STREAM, 0);

    struct sockaddr_in servaddr;
    bzero(&servaddr, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    servaddr.sin_port = htons(kServPort);

    int ret1 = bind(listenfd, (struct sockaddr *)&servaddr, sizeof(servaddr));
    if(ret1 < 0) error(1, errno, "graceserver: bind failed");

    int ret2 = listen(listenfd, 1024);
    if(ret2 < 0) error(1, errno, "graceserver: listen failed");

    signal(SIGINT, sig_int);
    signal(SIGPIPE, sig_pipe); // signal(SIGPIPE, SIG_DFL);

    int connfd;
    struct sockaddr_in cliaddr;
    socklen_t clilen = sizeof(cliaddr);
    if((connfd = accept(listenfd, (struct sockaddr*)&cliaddr, &clilen)) < 0)
        error(1, errno, "graceserver: accept failed");
    
    char message[kMaxLine];
    for(;;) {
        int n = read(connfd, message, kMaxLine);
        if(n < 0) error(1, errno, "graceserver: error read");
        else if(n == 0) error(1, 0, "graceserver: client closed\n");
        message[n] = 0;
        printf("graceserver: received %d bytes: %s\n", n, message);
        count++;

        char send_line[kMaxLine];
        sprintf(send_line, "hi, %s", message);
        sleep(10);

        int write_n = send(connfd, send_line, strlen(send_line), 0);
        printf("graceserver: send bytes: %zu\n", write_n);
        if(write_n < 0) error(1, errno, "graceserver: error write");
    }
}