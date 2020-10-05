#ifndef SERVER_H
#define SEVER_H
#include <sys/socket.h>
//#include <netinet/in.h>
#include <arpa/inet.h>
#include "logger.h"
#include "epoller.h"

class Server {
private:
    bool InitSocket_();
private:
    int port_;
    int timeoutMs_;
    bool isClose_;
    int listenFd_;
    uint32_t listenEvent_;
};

#endif