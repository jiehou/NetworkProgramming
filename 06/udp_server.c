#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <errno.h>
#include <signal.h>

const int kServPort = 43211;
const int kMaxLine = 4096;
static int count;

/* respond to CTRL+C */
static void recvfrom_int(int signo) {
    printf("\nreceived %d datagrams\n", count);
    exit(0);
}

int main(int argc, char** argv) {
    int socket_fd;
    socket_fd = socket(AF_INET, SOCK_DGRAM, 0);
    
    struct sockaddr_in servaddr;
    bzero(&servaddr, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    servaddr.sin_port = htons(kServPort);

    bind(socket_fd, (struct sockaddr*)&servaddr, sizeof(servaddr));
    
    socklen_t clilen; // cli: client
    char message[kMaxLine];
    count = 0;
    signal(SIGINT, recvfrom_int);

    struct sockaddr_in cliaddr;
    clilen = sizeof(cliaddr);
    for(;;) {
        int n = recvfrom(socket_fd, message, kMaxLine, 0, (struct sockaddr*)&cliaddr, &clilen);
        printf("[D] n: %d\n", n);
        message[n] = 0;
        printf("received %d bytes: %s\n", n, message);

        char send_line[kMaxLine];
        sprintf(send_line, "Hi, %s", message);
        sendto(socket_fd, send_line, strlen(send_line), 0, (struct sockaddr*)&cliaddr, clilen);
        count++;
    }
}