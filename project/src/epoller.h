#ifndef EPOLLER_H
#define EPOLLER_H
#include <sys/epoll.h> // epoll_ctl
#include <errno.h> 
#include <cassert>
#include <vector>
using std::vector;

/**
 * epoll
 * Advantages: 
 * 1) better performance than select/poll
 * 2) permit both level-triggered and edge-triggered notifications
 * APIs:
 * 1) int epoll_create(int size): return file descriptor on success, or -1 on error
 * 2) int epoll_ctl(int epfd, int op, int fd, struct epoll_event* cv): return 0 on success, or -1 on error
 * 3) int epoll_wait(int epfd, struct epoll_event *evlist, int maxevents, int timeout): return number of
 * ready file descriptors, 0 on timeout, -1 on error 
*/
class Epoller {
public:
    explicit Epoller(int maxEvents = 512);
    virtual ~Epoller();
    bool AddFd(int fd, uint32_t events);
    bool ModifyFd(int fd, uint32_t events);
    bool DeleteFd(int fd);
    int Wait(int timeoutMs = -1);
    int GetEventFd(size_t idx) const;
    uint32_t GetEvents(size_t idx) const;
private:
    int epFd_;
    vector<struct epoll_event> events_;
};
#endif