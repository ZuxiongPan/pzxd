#include "core/msg_queue.h"
#include "common/log.h"

#include <stdlib.h>

static struct msg_queue queues[MODULE_ID_MAX] = { 0 };
static bool queue_alive = false;

static inline void release_resource(int mtx_index, int cond_index) {
    int i = 0;
    for(i = 0; i < mtx_index; i++) {
        pthread_mutex_destroy(&queues[i].mutex);
    }
    for(i = 0; i < cond_index; i++) {
        pthread_cond_destroy(&queues[i].cond);
    }

    return ;
}

int msg_queue_init(void) {
    if(queue_alive) {
        log_error("Message Queue already initialized\n");
        return -EAGAIN;
    }

    for(int i = 0; i < MODULE_ID_MAX; i++) {
        queues[i].head = 0;
        queues[i].tail = 0;
        queues[i].count = 0;
        memset(&queues[i].messages, 0, sizeof(queues[i].messages));

        if(pthread_mutex_init(&queues[i].mutex, NULL)) {
            log_error("Failed to initialize mutex for module %d\n", i);
            release_resource(i, i);
            return -EFAULT;
        }
        if(pthread_cond_init(&queues[i].cond, NULL)) {
            log_error("Failed to initialize condition for module %d\n", i);
            release_resource(i + 1, i);
            return -EFAULT;
        }
    }
    queue_alive = true;
    
    log_info("Message Queue initialized successfully\n");
    return SUCCESS;
}

void msg_queue_destroy(void) {
    if(!queue_alive) {
        log_info("Message Queue not initialized\n");
        return ;
    }

    for(int i = 0; i < MODULE_ID_MAX; i++) {
        pthread_mutex_lock(&queues[i].mutex);
        pthread_cond_broadcast(&queues[i].cond);
        pthread_mutex_unlock(&queues[i].mutex);
    }

    queue_alive = false;
    release_resource(MODULE_ID_MAX, MODULE_ID_MAX);

    return ;
}

int send_message(const struct message *msg_in) {
    if(NULL == msg_in || msg_in->dst <= MODULE_ID_NONE || msg_in->dst >= MODULE_ID_MAX) {
        log_error("Invalid message\n");
        return -EINVAL;
    }

    if(!queue_alive) {
        log_error("Message Queue is not ready\n");
        return -ENOTCONN;
    }

    int dst = msg_in->dst;
    pthread_mutex_lock(&queues[dst].mutex);
    if(queues[dst].count >= MSGQUEUE_MAXCOUNT) {
        log_error("Destination module %d queue full\n", dst);
        pthread_mutex_unlock(&queues[dst].mutex);
        return -EOVERFLOW;
    }

    memcpy(&queues[dst].messages[queues[dst].tail], msg_in, sizeof(struct message));
    queues[dst].tail = (queues[dst].tail + 1) % MSGQUEUE_MAXCOUNT;
    queues[dst].count++;
    pthread_cond_signal(&queues[dst].cond);
    pthread_mutex_unlock(&queues[dst].mutex);

    return SUCCESS;
}

int receive_message(enum module_id self, struct message *msg_out) {
    if(NULL == msg_out || self <= MODULE_ID_NONE || self >= MODULE_ID_MAX) {
        log_error("Invalid receiver\n");
        return -EINVAL;
    }

    pthread_mutex_lock(&queues[self].mutex);
    while(queues[self].count == 0 && queue_alive) {
        pthread_cond_wait(&queues[self].cond, &queues[self].mutex);
    }
    
    if(!queue_alive) {
        pthread_mutex_unlock(&queues[self].mutex);
        return -ENOTCONN;
    }

    memcpy(msg_out, &queues[self].messages[queues[self].head], sizeof(struct message));
    queues[self].head = (queues[self].head + 1) % MSGQUEUE_MAXCOUNT;
    queues[self].count--;
    pthread_mutex_unlock(&queues[self].mutex);

    return SUCCESS;
}
