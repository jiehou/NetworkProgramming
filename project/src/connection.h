#ifndef CONNECTION_H
#define CONNECTION_H

#include "buffer.h"
#include <arpa/inet.h>

class Connection {
public:
    Connection() = default;
    void Init(int fd, const sockaddr_in& addr);
private:
    int fd_;
    struct sockaddr_in addr_;
};
#endif