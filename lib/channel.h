#ifndef CHANNEL_H
#define CHANNEL_H

#include "event_loop.h"

#define EVENT_TIMEOUT   0x01
#define EVENT_READ      0x02 // wait for socket or fd to become readable
#define EVENT_WRITE     0x04 // wait for socket or fd to become writable
#define EVENT_SIGNAL    0x08 // wait for a POSIX signal to be raised

typedef int(*eventReadCallback)(void* data);
typedef int(*eventWriteCallback)(void* data);

struct channel{
    int fd;
    int events;
    eventReadCallback readCallback;
    eventWriteCallback writeCallback;
    void* data; // it can be event_loop, tcp_server, or tcp_connection
};

struct channel* channel_new(
    int fd, int events, eventReadCallback rcallback, 
    eventWriteCallback wcallback, void* data
);

int channel_write_event_enabled(struct channel* ch);
int channel_write_event_enable(struct channel* ch);
int channel_write_event_disable(struct channel* ch);
#endif