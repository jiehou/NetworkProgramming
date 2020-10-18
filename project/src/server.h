#ifndef SERVER_H
#define SEVER_H
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include "logger.h"
#include "epoller.h"
#include "heaptimer.h"
#include "threadpool.h"

class Server {
public:
    Server(size_t port, size_t timeoutMs, size_t numThreads);
    virtual ~Server();
    void Start(); // start the server
private:
    bool InitSocket_(); 
    void ManageListen_();
    void AddConnection_();
    void ManageRead_();
    void ManageWrite_();
    void OnListen_();
    void OnWrite_();
private:
    size_t port_; // port number
    size_t timeoutMs_; // expected time that an event to be processed
    bool isClose_;
    int listenFd_; // file descriptor for listening
    uint32_t listenEvent_; // listen event
    uint32_t connEvent_; // connection event

    std::unique_ptr<HeapTimer> timer_;
    std::unique_ptr<ThreadPool> threadpool_;
    std::unique_ptr<Epoller> epoller_;
    
};

#endif