#include "connection.h"

void Connection::Init(int fd, const sockaddr_in& addr) {
    fd_ = fd;
    addr_ = addr;
}