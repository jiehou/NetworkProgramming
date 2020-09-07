#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <errno.h>
#include <signal.h>

const int kMaxLine   = 4096;
const int kServPort  = 54321;

int main(int argc, char** argv) {
    if(argc != 2) error(1, 0, "usage: graceclient <ip_address>");
    
    int sockfd;
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    struct sockaddr_in servaddr;
    bzero(&servaddr, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(kServPort);
    inet_pton(AF_INET, argv[1], &servaddr.sin_addr);

    socklen_t serv_len = sizeof(servaddr);
    int conn_ret = connect(sockfd, (struct sockaddr*)&servaddr, serv_len);
    if(conn_ret < 0) error(1, errno, "graceclient: connect failed");
    char send_line[kMaxLine], recv_line[kMaxLine];
    int n;

    fd_set readmask;
    fd_set allreads;
    FD_ZERO(&allreads);
    FD_SET(0, &allreads);
    FD_SET(sockfd, &allreads);
    for(;;) {
        readmask = allreads;
        int rc = select(sockfd+1, &readmask, NULL, NULL, NULL);
        if(rc <= 0) error(1, errno, "graceclient: select failed");
        
        if(FD_ISSET(sockfd, &readmask)) {
            n = read(sockfd, recv_line, kMaxLine);
            if(n < 0) error(1, errno, "graceclient: read error");
            else if(n == 0) {
                error(1 , 0, "graceclient: server terminated\n");
            }
            recv_line[n] = 0;
            fputs(recv_line, stdout);
            fputs("\n", stdout);
        }

        if(FD_ISSET(0, &readmask)) {
            if(fgets(send_line, kMaxLine, stdin) != NULL) {
                if(strncmp(send_line, "shutdown", 8) == 0) {
                    FD_CLR(0, &allreads);
                    if(shutdown(sockfd, 1)) error(1, errno, "graceclient: shutdown failed");
                } else if(strncmp(send_line, "close", 5) == 0) {
                    FD_CLR(0, &allreads);
                    if(close(sockfd)) error(1, errno, "graceclient: close failed");
                    sleep(6);
                    exit(0);
                } else {
                    int i = strlen(send_line);
                    if(send_line[i-1]=='\n') send_line[i-1] = 0;
                    printf("graceclient: now sending %s\n", send_line);
                    size_t ret = write(sockfd, send_line, strlen(send_line));
                    if(ret < 0) error(1, errno, "graceclient: write failed");
                    printf("graceclient: send bytes %zu\n", ret);
                }
            }
        }
    }
}