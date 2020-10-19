#include "connection.h"
#include <iostream>
using std::cout;
using std::endl;

void Connection::Init(int fd, const sockaddr_in& addr, bool isEt) {
    fd_ = fd;
    addr_ = addr;
    isEt_ = isEt;
}

int Connection::GetFd() const {
    return fd_;
}

sockaddr_in Connection::GetAddr() const {
    return addr_;
}

ssize_t Connection::Read(int* retErrno) {
    ssize_t len = -1;
    //do 
    {
        len = buf_.ReadFd(fd_, retErrno);
        //cout << "[D] Connection Read len: " << len << ", fd_: " << fd_ << endl;
        //if(len <= 0) break;
    }
    //while(isEt_);
    return len;
}

ssize_t Connection::Write(int* retErrno) {
    ssize_t len = -1;
    //do {
        len = buf_.WriteFd(fd_, retErrno);
        //if(len <= 0) break;
    //}
    //while(isEt_);
    return len;
}