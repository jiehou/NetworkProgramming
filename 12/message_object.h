#ifndef MESSAGE_OBJECT_H
#define MESSAGE_OBJECT_H
#include <stdint.h>

#define MSG_PING    1
#define MSG_PONG    2
#define MSG_TYPE1   11
#define MSG_TYPE2   12

typedef struct {
    u_int32_t type;
    char data[1024];
} MessageObject;

#endif