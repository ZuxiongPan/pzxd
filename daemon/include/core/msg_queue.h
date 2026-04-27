#ifndef __MSG_QUEUE_H__
#define __MSG_QUEUE_H__

#include "modules/module_ids.h"
#include "core/message.h"

#include <stdbool.h>
#include <pthread.h>

#define MSGQUEUE_MAXCOUNT 8

struct msg_queue {
    int head;
    int tail;
    int count;
    pthread_mutex_t mutex;
    pthread_cond_t cond;
    struct message messages[MSGQUEUE_MAXCOUNT];
};

int msg_queue_init(void);
void msg_queue_destroy(void);
int send_message(const struct message *msg_in);
int receive_message(enum module_id self, struct message *msg_out);

#endif