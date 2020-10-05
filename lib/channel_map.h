#ifndef CHANNEL_MAP_H
#define CHANNEL_MAP_H
#include "channel.h"

struct channel_map {
    void** channels;
    int numChannels;
};

int cmap_make_space(struct channel_map* cmap, int numChannels, int channelSize);
void cmap_init(struct channel_map* cmap);
void cmap_clear(struct channel_map* cmap);
#endif