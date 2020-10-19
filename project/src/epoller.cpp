#include "epoller.h"
#include "logger.h"

Epoller::Epoller(int maxEvents) : epFd_(epoll_create(256)), events_(maxEvents) {
    //LOG_INFO("Epoller epFd_ {}", epFd_);
    assert(epFd_ >= 0 && events_.size() > 0);
}

Epoller::~Epoller() { close(epFd_); }

/**
 * @return
 * true: success
 * false: error
*/
bool Epoller::AddFd(int fd, uint32_t events) {
    if(fd < 0) return false;
    epoll_event ev = {0};
    ev.events = events;
    ev.data.fd = fd;
    return (0 == epoll_ctl(epFd_, EPOLL_CTL_ADD, fd, &ev)); //epoll_ctl: returns 0 on success, or -1 error
}

bool Epoller::ModifyFd(int fd, uint32_t events) {
    if(fd < 0) return false;
    epoll_event ev = {0};
    ev.events = events;
    ev.data.fd = fd;
    return (0 == epoll_ctl(epFd_, EPOLL_CTL_MOD, fd, &ev));
}

bool Epoller::DeleteFd(int fd) {
    if(fd < 0) return false;
    epoll_event ev = {0};
    return (0 == epoll_ctl(epFd_, EPOLL_CTL_DEL, fd, &ev));
}

int Epoller::Wait(int timeoutMs) {
    /**
     * API: int epoll_wait(int epfd, struct epoll_event* evlist, int maxevents, int timeout)
     * @return: the above api return the number of read file descriptors, 0 on timeout, or -1 on error
     * @param: 
     * timeout(-1): block until an event occurs for one of the file descriptors in the interest list for epfd or
     * until a signal is caught
     * timeout(0): perform a nonblocking check to see which events are currently available
     * timeout(>0): block for up to timeout milliseconds  
    */
    return epoll_wait(epFd_, &events_[0], events_.size(), timeoutMs);
}

int Epoller::GetEventFd(size_t idx) const {
    assert(idx >= 0 && idx < events_.size());
    return events_[idx].data.fd;
}

uint32_t Epoller::GetEvents(size_t idx) const {
    assert(idx >= 0 && idx < events_.size());
    return events_[idx].events;
}