#ifndef SERVER_H
#define SERVER_H
#include <unordered_map>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include "logger.h"
#include "epoller.h"
#include "heaptimer.h"
#include "threadpool.h"
#include "connection.h"
using std::unordered_map;

class Server {
public:
    Server(size_t port, size_t timeoutMs, size_t numThreads);
    virtual ~Server() {};
    void Start(); // start the server
private:
    bool InitSocket_(); 
    void ManageListen_();
    void AddConnection_(int fd, const sockaddr_in& addr);
    void ManageRead_(Connection* conn);
    void ManageWrite_(Connection* conn);
    void OnListen_(Connection* conn);
    void OnWrite_(Connection* conn);
    void OnRead_(Connection* conn);
    void SendMsgToClient_(int fd, const char* msg);
    void CloseConnection_(Connection* conn);
    void ExtendTimer_(Connection* conn);
private:
    size_t port_; // port number
    size_t timeoutMs_; // expected time that an event to be processed
    bool isClose_;
    int listenFd_; // file descriptor for listening
    uint32_t listenEvent_; // listen event
    uint32_t connEvent_; // connection event
    size_t numConnections_;

    std::unique_ptr<HeapTimer> timer_;
    std::unique_ptr<ThreadPool> threadpool_;
    std::unique_ptr<Epoller> epoller_;
    unordered_map<int, Connection> connLookup_; // lookup for connection <fd, connection>
};
#endif