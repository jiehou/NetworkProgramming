#include <assert.h>
#include "channel_map.h"

/*channelSize: sizeof(struct channel*)*/
int cmap_make_space(struct channel_map* cmap, struct channel_map* cmap, int numChannes) {
    if(cmap->numChannels <= numChannes) {
        int tmpSize = cmap->numChannels ? cmap->numChannels : 32;
        void** tmp;
        int channelSize = sizeof(struct channel*);
        while(tmpSize <= numChannes) tmpSize <<= 1; // each loop: tmpSize *= 2
        tmp = (void**) realloc(cmap->channels, tmpSize * channelSize);
        if(tmp == NULL) return -1;
        memset(&tmp[cmap->numChannels], 0, (tmpSize - cmap->numChannels) * channelSize);
        cmap->numChannels = tmpSize;
        cmap->channels = tmp;
    }
    return 0;
}

void cmap_init(struct channel_map* cmap) {
    cmap->channels = NULL;
    cmap->numChannels = 0;
}

void cmap_clear(struct channel_map* cmap) {
    if(cmap->channels != NULL) {
        for(int i = 0; i < cmap->numChannels; ++i) {
            if(cmap->channels[i] != NULL) {
                free(cmap->channels[i]);
                cmap->channels[i] = NULL;
            }
        }//for
        free(cmap->channels);
        cmap->channels = NULL;
    }
    cmap->numChannels = 0;
}