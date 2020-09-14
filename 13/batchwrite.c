#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <errno.h>
#include <signal.h>

const int kMaxLine      = 4096;
const int kServPort     = 54321;

int main(int argc, char** argv) {
    if(argc != 2) error(1, 0, "usage: batchwrite <ip_address>");
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);

    struct sockaddr_in servaddr;
    bzero(&servaddr, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(kServPort);
    inet_pton(AF_INET, argv[1], &servaddr.sin_addr);

    socklen_t servlen = sizeof(servaddr);
    int conn_ret = connect(sockfd, (struct sockadd*)&servaddr, servlen);
    if(conn_ret < 0) error(1, errno, "batchwrite: connect failed");

    char buf[128];
    struct iovec iov[2];
    char *send_first = "hello,";
    iov[0].iov_base = send_first;
    iov[0].iov_len = strlen(send_first);
    iov[1].iov_base = buf;
    while(fgets(buf, sizeof(buf), stdin) != NULL) {
        int buflen = strlen(buf);
        iov[1].iov_len = buflen;
        int n = htonl(buflen);
        if(writev(sockfd, iov, 2) < 0) error(1, errno, "batchwrite: writev failure");
    }
    exit(0);
}