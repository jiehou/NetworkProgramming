#ifndef UTILS_H
#define UTILS_H
#include <cassert>
#include <fcntl.h>

const size_t kMaxConnections = 1024;
const char* kServerBusy = "Server is busy";

inline int SetFdNonblock(int fd) {
    assert(fd >= 0);
    //fcntl(fd, F_SETFL, fcntl(fd, F_GETFL) | O_NONBLOCK)
    return fcntl(fd, F_SETFL, fcntl(fd, F_GETFD, 0) | O_NONBLOCK);
}
#endif
