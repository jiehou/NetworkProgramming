#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <errno.h>
#include <signal.h>
#include <poll.h>
#include <fcntl.h>

#define kMaxLine  4096

const int kInitSize     = 128;
const int kServPort     = 54321;

char rot13_char(char c) {
    if ((c >= 'a' && c <= 'm') || (c >= 'A' && c <= 'M'))
        return c + 13;
    else if ((c >= 'n' && c <= 'z') || (c >= 'N' && c <= 'Z'))
        return c - 13;
    else
        return c;
}

struct Buffer {
    int connfd;
    char elems[kMaxLine];
    size_t wrt_idx; // write index
    size_t read_idx; // read index
    int readable;
};

struct Buffer* alloc_buffer() {
    struct Buffer *buffer = malloc(sizeof(struct Buffer));
    if(!buffer) return NULL;
    buffer->connfd = 0;
    buffer->read_idx = 0;
    buffer->wrt_idx = 0;
    buffer->readable = 0;
    return buffer;
}

void free_buffer(struct Buffer* buffer) {
    free(buffer);
}

int onsocket_read(int fd, struct Buffer* buffer) {
    char buf[1024];
    int i;
    ssize_t res;
    while (1) {
        res = recv(fd, buf, sizeof(buf), 0);
        if(res <= 0) break;
        for(i = 0; i < res; ++i) {
            if(buffer->wrt_idx < sizeof(buffer->elems))
                buffer->elems[buffer->wrt_idx++] = buf[i];
            if(buf[i] == '\n') buffer->readable = 1; 
        }   
    }
    if(res == 0) return 1;
    else if(res < 0) {
        if(errno == EAGAIN) return 0;
        return -1;
    }
    return 0;
}

int onsocket_write(int fd, struct Buffer* buffer) {
    while(buffer->read_idx < buffer->wrt_idx) {
        ssize_t res = send(fd, buffer->elems+buffer->read_idx, buffer->wrt_idx - buffer->read_idx, 0);
        if(res < 0) {
            if(errno == EAGAIN) return 0;
            return -1;
        }
        buffer->read_idx += res;
    }
    if(buffer->read_idx == buffer->wrt_idx) {
        buffer->read_idx = 0;
        buffer->wrt_idx = 0;
    }
    buffer->readable = 0;
    return 0;
}

int tcp_nonblocking_server_listen(int port) {
    int listenfd = socket(AF_INET, SOCK_STREAM, 0);
    fcntl(listenfd, F_SETFL, O_NONBLOCK);

    struct sockaddr_in servaddr;
    bzero(&servaddr, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    servaddr.sin_port = htons(kServPort);

    int on = 1;
    setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on));

    int ret1 = bind(listenfd, (struct sockaddr*)&servaddr, sizeof(servaddr));
    if(ret1 < 0) error(1, errno, "bind failed");

    int ret2 = listen(listenfd, 1024);
    if(ret2 < 0) error(1, errno, "listen failed");
    return listenfd;
}

int main(int argc, char** argv) {
    int listenfd;
    int i, maxfd;
    struct Buffer* buffers[kInitSize];
    for(int i = 0; i < kInitSize; ++i)
        buffers[i] = alloc_buffer();
    
    listenfd = tcp_nonblocking_server_listen(kServPort);

    fd_set readset, writeset, exset;
    FD_ZERO(&readset);
    FD_ZERO(&writeset);
    FD_ZERO(&exset);

    while(1) {
        maxfd = listenfd;
        FD_ZERO(&readset);
        FD_ZERO(&writeset);
        FD_ZERO(&exset);

        FD_SET(listenfd, &readset);
        for(i = 0; i < kInitSize; ++i) {
            if(buffers[i]->connfd > 0) {
                if(buffers[i]->connfd > maxfd) maxfd = buffers[i]->connfd;
                if(buffers[i]->readable) FD_SET(buffers[i]->connfd, &writeset);
            }
        }

        if(select(maxfd+1, &readset, &writeset, &exset, NULL) < 0) error(1, errno, "select error");

        if(FD_ISSET(listenfd, &readset)) {
            printf("listening socket readable\n");
            sleep(5);
            struct sockaddr_storage ss;
            socklen_t ss_len = sizeof(ss);
            int fd = accept(listenfd, (struct sockaddr*)&ss, &ss_len);
            if(fd < 0) error(1, errno, "accept failed");
            else if(fd > kInitSize) {
                error(1, 0, "too many connections");
                close(fd);
            }
            else {
                fcntl(fd, F_SETFL, O_NONBLOCK);
                if(buffers[fd]->connfd == 0) buffers[fd]->connfd = fd;
                else error(1, 0, "too many connections");
            }
        }

        for(i = 0; i < maxfd + 1; ++i) {
            int r = 0;
            if(i == listenfd) continue;
            if(FD_ISSET(i, &readset)) r = onsocket_read(i, buffers[i]);
            if(r == 0 && FD_ISSET(i, &writeset)) r = onsocket_write(i, buffers[i]);
            if(r) {
                buffers[i]->connfd = 0;
                close(i);
            }
        }
    }
}