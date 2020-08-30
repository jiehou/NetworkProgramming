#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <errno.h>
#include <signal.h>
#include <sys/un.h> /* domain packets */

const int kBufferSize = 4096;
const int kMaxLine    = 4096;

int main(int argc, char** argv) {
    if(argc != 2) error(1, 0, "usage: stream_client <local_path>");
    int sockfd;
    struct sockaddr_un servaddr;
    sockfd = socket(AF_LOCAL, SOCK_STREAM, 0);
    if(sockfd < 0) error(1, errno, "stream_client: create socket failed");
    bzero(&servaddr, sizeof(servaddr));
    servaddr.sun_family = AF_LOCAL;
    strcpy(servaddr.sun_path, argv[1]);
    if(connect(sockfd, (struct sockaddr*)&servaddr, sizeof(servaddr)) < 0) error(1, errno, "stream_client: connect failed");

    char send_line[kMaxLine];
    bzero(send_line, kMaxLine);
    char recv_line[kMaxLine];
    while (fgets(send_line, kMaxLine, stdin) != NULL) {
        int nbytes = sizeof(send_line);
        if(write(sockfd, send_line, nbytes) != nbytes) error(1, errno, "stream_client: write error");
        if(read(sockfd, recv_line, kMaxLine) == 0)
            error(1, errno, "stream_client: server terminated prematurely");
        //fputs(recv_line, stdout);
        printf("%s", recv_line);
    }
    exit(0);
}