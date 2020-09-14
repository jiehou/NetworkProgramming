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
static void recvfrom_int(int signo) {
    printf("ucserver: received %d datagrams\n", count);
    exit(0);
}

int main(int argc, char** argv) {
    count = 0;
    int sockfd = socket(AF_INET, SOCK_DGRAM, 0);

    struct sockaddr_in servaddr;
    bzero(&servaddr, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    servaddr.sin_port = htons(kServPort);
    bind(sockfd, (struct sockaddr*)&servaddr, sizeof(servaddr));
    
    char message[kMaxLine];
    message[0] = 0;
    struct sockaddr_in cliaddr;
    socklen_t clilen = sizeof(cliaddr);
    int n = recvfrom(sockfd, message, kMaxLine, 0, (struct sockaddr*)&cliaddr, &clilen);
    if(n < 0) error(1, errno, "ucserver: recvfrom failed");
    message[n] = 0;
    printf("ucserver: received %d bytes, %s\n", n, message);

    int conn_ret = connect(sockfd, (struct sockaddr *)&cliaddr, clilen);
    printf("[D] conn_ret: %d", conn_ret);
    if(conn_ret) error(1, errno, "ucserver: connect failed");
    while(strncmp(message, "goodbye", 7) != 0) {
        char send_line[kMaxLine];
        sprintf(send_line, "hi, %s", message);
        size_t ret = send(sockfd, send_line, strlen(send_line), 0);
        if(ret < 0) error(1, errno, "ucserver: send failed");
        printf("ucserver: send bytes %zu", ret);

        size_t rc = recv(sockfd, message, kMaxLine, 0);
        if(rc < 0) error(1, errno, "ucserver: recv failed");
        count++;
    }
    exit(0);
}