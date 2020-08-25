#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>

size_t readn(int fd, void* buffer, size_t size) {
	char* buffer_ptr = buffer;
	size_t len = size;
	while(len > 0) {
		int res = read(fd, buffer_ptr, len);
		if(res < 0) 
	}
}

void ReadData(int sockfd) {
    ssize_t n;
    char buf[1024];
    int time = 0;
    for(;;) {
        fprintf(stdout, "block in read\n");
        if((n = readn(sockfd, buf, 1024)) == 0) return;
        time++;
        fprintf(stdout, "1K read for %d\n", time);
        usleep(1000);
    }
}

int main(int argc, char** argv) {
    int listenfd, connfd;
    socklen_t clilen;
    struct sockaddr_in cliaddr, seraddr;

    listenfd = socket(AF_INET, SOCK_STREAM, 0);
    if(listenfd < 0) {
        perror("server socket");
        exit(EXIT_FAILURE);
    }

    bzero(&seraddr, sizeof(seraddr));
    seraddr.sin_family = AF_INET;
    seraddr.sin_port = htons(54321);
    seraddr.sin_addr.s_addr = htonl(INADDR_ANY);
    bind(listenfd, (struct sockaddr_in*)&seraddr, sizeof(seraddr));
    listen(listenfd, 100); // backlog set to 100
    for(;;) {
        clilen = sizeof(cliaddr);
        connfd = accept(listenfd, (struct sockaddr_in*)&cliaddr, &clilen);
        ReadData(connfd);
        close(connfd);
    }
}
