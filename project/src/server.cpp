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
    if(!InitSocket_()) isClose_ = true; // true: close, false: run server
}

//@TODO: Start
void Server::Start() {
    if(isClose_) return;
    int timeMs = -1;
    LOG_INFO("Server starts running");
    while(!isClose_) {
        if(timeoutMs_ > 0) timeMs = timer_->GetNextTick();
        int eventCount = epoller_->Wait(timeMs);
        cout << "[D] eventCount: " << eventCount << endl;
        for(int i = 0; i < eventCount; ++i) {
            int fd = epoller_->GetEventFd(i);
            uint32_t tmpEvents = epoller_->GetEvents(i);
            if(fd == listenFd_) {
                cout << "[D] ManageListen_\n";
                ManageListen_();
                 cout << "[D] ManageListen_1\n";
            }
            else if(tmpEvents & (EPOLLRDHUP | EPOLLHUP | EPOLLERR)) {
                assert(connLookup_.count(fd) > 0);
                CloseConnection_(&connLookup_[fd]);
            }
            else if(tmpEvents & EPOLLIN) {
                assert(connLookup_.count(fd) > 0);
                ManageRead_(&connLookup_[fd]);
            }
            else if(tmpEvents & EPOLLOUT) {
                assert(connLookup_.count(fd) > 0);
                ManageWrite_(&connLookup_[fd]);
            }
            else LOG_ERROR("Unexpected event");
        }
    }
}

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
    ret = listen(listenFd_, 1024);
    if(ret < 0) {
        close(listenFd_);
        LOG_ERROR("Listen error");
        return false;
    }
    if(!epoller_->AddFd(listenFd_, listenEvent_ | EPOLLIN)) {
        close(listenFd_);
        LOG_ERROR("Epoller add listenFd error");
        return false;
    }
    SetFdNonblock(listenFd_);
    LOG_INFO("Server listens on port {}", port_);
    return true;
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
    numConnections_++; // increase numConnections_
    if(timeoutMs_ > 0)
        timer_->Add(fd, timeoutMs_, std::bind(&Server::CloseConnection_, this, &connLookup_[fd]));
    epoller_->AddFd(fd, EPOLLIN || connEvent_);
    SetFdNonblock(fd);
    LOG_INFO("Connection with fd: {} is established", fd);
}

void Server::ExtendTimer_(Connection* conn) {
    if(!conn) return;
    if(timeoutMs_ > 0)
        timer_->Update(conn->GetFd(), timeoutMs_);
}

void Server::ManageRead_(Connection* conn) {
    if(!conn) return;
    ExtendTimer_(conn);
    threadpool_->AddTask(std::bind(&Server::OnRead_, this, conn));
}

void Server::OnRead_(Connection* conn) {
    if(!conn) return;
    int ret = -1, readErrno = 0;
    ret = conn->Read(&readErrno);
    if(ret <= 0 && readErrno != EAGAIN) {
        CloseConnection_(conn);
        return;
    }
    // @NOTE: simple echo server
    epoller_->ModifyFd(conn->GetFd(), connEvent_ | EPOLLOUT); 
}

void Server::ManageWrite_(Connection* conn) {
    if(!conn) return;
    ExtendTimer_(conn);
    threadpool_->AddTask(std::bind(&Server::OnWrite_, this, conn));
}

void Server::OnWrite_(Connection* conn) {
    if(!conn) return;
    int ret = -1, writeErrno = 0;
    ret = conn->Write(&writeErrno);
    if(ret < 0 && writeErrno == EAGAIN) {
        epoller_->ModifyFd(conn->GetFd(), connEvent_ | EPOLLOUT);
    }
    else {
        epoller_->ModifyFd(conn->GetFd(), connEvent_ | EPOLLIN);
    }
}

void Server::SendMsgToClient_(int fd, const char* msg) {
    assert(fd >= 0);
    int ret = send(fd, msg, strlen(msg), 0);
    if(ret < 0) {
        close(fd);
        LOG_ERROR("SendMsgToClient error");
    }
}

void Server::CloseConnection_(Connection* conn) {
    if(!conn) return;
    int fd = conn->GetFd();
    LOG_INFO("Close connection with fd: {}", fd);
    epoller_->DeleteFd(fd);
    close(fd);
    numConnections_--; // decrease numConnection_
}