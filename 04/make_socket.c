#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>

int makeSocket(uint16_t port) {
    int sock;
    // create IPv4 socket
    sock = socket(PF_INET, SOCK_STREAM, 0);
    if(sock < 0) {
        perror("socket");
        exit(EXIT_FAILURE);
    }
    struct sockaddr_in servaddr;
    memset(&servaddr, 0, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(port);
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    if(bind(sock, (struct sockaddr_in*)&servaddr, sizeof(servaddr)) < 0) {
        perror("bind");
        exit(EXIT_FAILURE);
    }
    return sock;
}

int main(int argc, char **argv) {
    int sockfd = makeSocket(54321);
    printf("#[I] sockfd: %d\n", sockfd);
}
