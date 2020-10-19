#ifndef CONNECTION_H
#define CONNECTION_H

#include "buffer.h"
#include <arpa/inet.h>

class Connection {
public:
    Connection() = default;
    void Init(int fd, const sockaddr_in& addr, bool isEt);
    int GetFd() const;
    sockaddr_in GetAddr() const;
    ssize_t Read(int* retErrno);
    ssize_t Write(int* retErrno);
private:
    int fd_;
    struct sockaddr_in addr_;
    bool isEt_; // is event-triggered?
    Buffer buf_;
};
#endif