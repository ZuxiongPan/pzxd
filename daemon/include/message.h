#pragma once

#include <stdint.h>
#include <string.h>

#define MAX_MSG_PAYLOAD_SIZE 256

enum msg_direction {
    MSG_DIR_INIT = 0,// initialize message direction
    MSG_DIR_INNER,  // in-process message
    MSG_DIR_OUTER,  // out-process message
};

typedef struct armd_msg {
    const char *src;
    const char *dst;
    /* bit 30-16: event type bit 15-0: event id */
    enum msg_direction dir;
    uint32_t event;
    uint32_t payload_len;
    char payload[MAX_MSG_PAYLOAD_SIZE];
} armd_msg_t;

#define MSG_EV_TYPE(event) ((event) & 0xffff0000u)
#define MSG_EV_ID(event) ((event) & 0x0000ffffu)

static inline void set_message(armd_msg_t *msg_in, const char *src, const char *dst,
                               enum msg_direction dir, uint32_t event,
                               const char *payload, uint32_t payload_len)
{
    msg_in->src = src;
    msg_in->dst = dst;
    msg_in->dir = dir;
    msg_in->event = event;
    msg_in->payload_len = payload_len > MAX_MSG_PAYLOAD_SIZE ? MAX_MSG_PAYLOAD_SIZE : payload_len;
    if (payload_len > 0 && NULL != payload)
    {
        memcpy(msg_in->payload, payload, msg_in->payload_len);
    }
}
