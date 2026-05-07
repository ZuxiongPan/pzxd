#ifndef __MESSAGE_H__
#define __MESSAGE_H__

#include "modules/module_ids.h"
#include <string.h>
#include <stdint.h>

#define MAX_MSG_SIZE 512

struct message {
    enum module_id src;
    enum module_id dst;
    uint32_t event; // bit 0-15: event id, bit 16-31: event type
    int payload_size;
    char payload[MAX_MSG_SIZE];
};

static inline void set_message(struct message *msg_ptr, enum module_id src,
    enum module_id dst, uint32_t event, int payload_size, const char *payload) 
{
    if (NULL == msg_ptr)
    {
        return ;
    }
    msg_ptr->src = src;
    msg_ptr->dst = dst;
    msg_ptr->event = event;
    msg_ptr->payload_size = payload_size > MAX_MSG_SIZE ? MAX_MSG_SIZE : payload_size;
    memcpy(msg_ptr->payload, payload, msg_ptr->payload_size);

    return ;
}

#define MSG_PAYLOAD(msg_ptr, type) ((type *)(msg_ptr)->payload)

#endif