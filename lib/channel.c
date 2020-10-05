#include "channel.h"

struct channel* channel_new(
    int fd, int events, eventReadCallback rcallback, 
    eventWriteCallback wcallback, void* data
) {
    struct channel* mch = (struct channel*) malloc(sizeof(struct channel));
    mch->fd = fd;
    mch->events = events;
    mch->readCallback = rcallback;
    mch->writeCallback = wcallback;
    mch->data = data;
    return mch;
}

int channel_write_event_enabled(struct channel* ch) {
    return ch->events & EVENT_WRITE;
}

int channel_write_event_enable(struct channel* ch) {
    struct event_loop* eloop = (struct event_loop*) ch->data;
    ch->events |= EVENT_WRITE;
    event_loop_update_channel_event(eloop, ch->fd, ch);
}

int channel_write_event_disable(struct channel* ch) {
    struct event_loop* eloop = (struct event_loop*) ch->data;
    ch->events &= ~EVENT_WRITE;
    event_loop_update_channel_event(eloop, ch->fd, ch);
}