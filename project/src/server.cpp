#include "server.h"

bool Server::InitSocket_() {
    if(port_ > 65535 || port_ < 1024) {
        LOG_ERROR("port: %d error", port_);
        return false;
    }
    int ret;
    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    addr.sin_port = htons(port_);
}

void Server::DealListen_() {
    struct sockaddr_in addr;
    socklen_t len = sizeof(addr);
    do {
        int fd = accept(listenFd_, (struct sockadrr*)&addr, &len);
        if(fd <= 0) return;
        else {
            //@TODO
        }
        AddClient_(fd, addr);
    }
    while(listenEvent_ & EPOLLET);
}

