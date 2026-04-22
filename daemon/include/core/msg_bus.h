#ifndef __MSG_BUS_H__
#define __MSG_BUS_H__

#include "modules/module_ids.h"
#include <string.h>

#define MAX_MSG_SIZE 1024

enum msg_type {
    MSG_TYPE_NONE = 0
};

struct process_message {
    enum module_id src;
    enum module_id dst;
    enum msg_type type;
    int payload_size;
    char payload[MAX_MSG_SIZE];
};

#define MSG_PAYLOAD(msg_ptr, type) ((type *)(msg_ptr)->payload)

static inline void set_message(struct process_message *msg_ptr, enum module_id src,
    enum module_id dst, enum msg_type type, int payload_size, const char *payload) {
    msg_ptr->src = src;
    msg_ptr->dst = dst;
    msg_ptr->type = type;
    msg_ptr->payload_size = payload_size;
    memcpy(msg_ptr->payload, payload, payload_size);

    return ;
}

int msg_bus_init(void);
void msg_bus_destroy(void);

#endif