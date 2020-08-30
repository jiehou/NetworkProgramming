#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <errno.h>

const int kServPort = 43211;
const int kMaxLine = 4096;

int main(int argc, char** argv) {
    if(argc != 2) error(1, 0, "usage: udp_client <IPAddress>");
    int socket_fd;
    socket_fd = socket(AF_INET, SOCK_DGRAM, 0);

    struct sockaddr_in servaddr;
    bzero(&servaddr, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(kServPort);
    inet_pton(AF_INET, argv[1], &servaddr.sin_addr);
    socklen_t servlen = sizeof(servaddr);

    struct sockaddr* replyaddr;
    replyaddr = malloc(servlen);

    char send_line[kMaxLine], recv_line[kMaxLine+1];
    socklen_t len;
    int n;
    while(fgets(send_line, kMaxLine, stdin) != NULL) {
        int i = strlen(send_line);
        if(send_line[i-1] == '\n') send_line[i-1] = 0;
        printf("now sending %s\n", send_line);
        size_t rt = sendto(socket_fd, send_line, strlen(send_line), 0, (struct sockaddr*)&servaddr, servlen);
        if(rt < 0) error(1, errno, "client: send failed");
        printf("send bytes: %zu \n", rt);

        len = 0;
        n = recvfrom(socket_fd, recv_line, kMaxLine, 0, replyaddr, &len);
        if(n < 0) error(1, errno, "client: recvfrom failed");
        recv_line[n] = 0;
        //fputs(recv_line, stdout);
        //fputs("\n", stdout);
        printf("now recv %s\n", recv_line);
    }
    exit(0);
}