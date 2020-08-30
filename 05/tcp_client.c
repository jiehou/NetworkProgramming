#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <errno.h>

const int kMessageSize = 10240;

void send_data(int sockfd) {
    char* query;
    query = malloc(kMessageSize+1);
    for(int i = 0; i < kMessageSize; ++i) query[i] = 'a';
    query[kMessageSize] = '\0';
    const char* cp;
    cp = query;
    size_t remaining = strlen(query);
    while (remaining) {
        int nwritten = send(sockfd, cp, remaining, 0);
        fprintf(stdout, "send into buffer %ld\n", nwritten);
        if(nwritten <= 0) {
            error(1, errno, "send failed");
            return;
        }
        remaining -= nwritten;
        cp += nwritten;
    }
}

int main(int argc, char** argv) {
    int sockfd;
    struct sockaddr_in servaddr;
    if(argc != 2) error(1, 0, "usage: tcp_client <ipaddress>");
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    bzero(&servaddr, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(54321);
    inet_pton(AF_INET, argv[1], &servaddr.sin_addr);
    int conn_ret = connect(sockfd, (struct sockaddr*)&servaddr, sizeof(servaddr));
    if(conn_ret < 0) error(1, errno, "connect failed");
    send_data(sockfd);
    exit(0);
}