#include "epoller.h"

Epoller::Epoller(int maxEvents) : epFd_(epoll_create1(256)), events_(maxEvents) {}

Epoller::~Epoller() { close(epFd_); }

bool Epoller::AddFd(int fd, uint32_t events) {
    if(fd < 0) return false;
    struct epoll_event ev;
    ev.events = events;
    ev.data.fd = fd;
    return 0 == epoll_ctl(epFd_, EPOLL_CTL_ADD, fd, &ev);
}

bool Epoller::ModifyFd(int fd, uint32_t events) {
    if(fd < 0) return false;
    struct epoll_event ev;
    ev.events = events;
    ev.data.fd = fd;
    return 0 == epoll_ctl(epFd_, EPOLL_CTL_MOD, &ev);
}

bool Epoller::DeleteFd(int fd) {
    if(fd < 0) return false;
    struct epoll_event ev;
    return 0 == epoll_ctl(epFd_, EPOLL_CTL_DEL, fd, &ev);
}

int Epoller::Wait(int timeoutMs = -1) {
    return epoll_wait(epFd_, &events_[0], events_.size(), timeoutMs);
}

int Epoller::GetEventFd(size_t idx) const {
    assert(idx >= 0 && idx < events_.size());
    return events_[i].data.fd;
}

uint32_t Epoller::GetEvents(size_t idx) const {
    assert(idx >= 0 && idx < events_.size());
    return events_[i].events;
}