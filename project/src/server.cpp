#include "server.h"
#include "utils.h"

Server::Server
(
    size_t port, size_t timeoutMs, size_t numThreads
) : port_(port), timeoutMs_(timeoutMs), isClose_(false), numConnections_(0) {
    timer_ = std::unique_ptr<HeapTimer>(new HeapTimer());
    threadpool_ = std::unique_ptr<ThreadPool>(new ThreadPool(numThreads));
    epoller_ = std::unique_ptr<Epoller>(new Epoller());
    // configure listenEvent_ and connEvent_
    listenEvent_ = EPOLLRDHUP | EPOLLET;
    connEvent_ = EPOLLONESHOT | EPOLLRDHUP | EPOLLET;
}

//@TODO: Start

/**
 * 1) create socket
 * 2) set socket options: reuse_addr, close elegently
 * 3) bind
 * 4) listen
*/
bool Server::InitSocket_() {
    if(port_ > 65535 || port_ < 1024) {
        LOG_ERROR("Port: {} error", port_);
        return false;
    }
    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    addr.sin_port = htons(port_);
    listenFd_ = socket(AF_INET, SOCK_STREAM, 0); // create IPV4 socket
    if(listenFd_ < 0) {
        LOG_ERROR("Create socket error");
        return false;
    }
    //@NOTE: close elegantly through setting socket option
    int ret;
    struct linger optLinger;
    optLinger.l_linger = 1;
    optLinger.l_onoff = 1;
    ret = setsockopt(listenFd_, SOL_SOCKET, SO_LINGER, &optLinger, sizeof(optLinger));
    if(ret < 0) {
        close(listenFd_);
        LOG_ERROR("Set socket options: LINGER error");
        return false;
    }
    //@NOTE: reuse address 
    int optVal = 1;
    ret = setsockopt(listenFd_, SOL_SOCKET, SO_REUSEADDR, &optVal, sizeof(int));
    if(ret < 0) {
        close(listenFd_);
        LOG_ERROR("Set socket options: REUSEADDR error");
        return false;
    }
    ret = bind(listenFd_, (struct sockaddr*)&addr, sizeof(addr)); // bind
    if(ret < 0) {
        close(listenFd_);
        LOG_ERROR("Bind error");
        return false;
    }
    if(!epoller_->AddFd(listenFd_, listenFd_ | EPOLLIN)) {
        close(listenFd_);
        LOG_ERROR("Epoller add listenFd error");
        return false;
    }
    SetFdNonblock(listenFd_);
    LOG_INFO("Server port {} listens", port_);
}

void Server::ManageListen_() {
    struct sockaddr_in addr;
    socklen_t len = sizeof(addr);
    do {
        int fd = accept(listenFd_, (struct sockaddr*)&addr, &len);
        if(fd <= 0) return;
        else if(numConnections_ > kMaxConnections){
            SendMsgToClient_(fd, kServerBusy);
            LOG_INFO(kServerBusy);
            return;
        }
        AddConnection_(fd, addr);
    }
    while(listenEvent_ & EPOLLET);
}

void Server::AddConnection_(int fd, const sockaddr_in& addr) {
    assert(fd >= 0);
    connLookup_[fd].Init(fd, addr);
    if(timeoutMs_ > 0)
        timer_->Add(fd, timeoutMs_, std::bind(&Server::CloseConnection_, this, &connLookup_[fd]));
    epoller_->AddFd(fd, EPOLLIN || connEvent_);
    SetFdNonblock(fd);
    LOG_INFO("Connection with fd: {} is established", fd);
}

void Server