#include "core/msg_bus.h"
#include "common/log.h"

#include <pthread.h>
#include <stdbool.h>

#define MODULE_MAXMSG_COUNT 8

struct msg_queue {
    int head;
    int tail;
    int count;
    struct process_message messages[MODULE_MAXMSG_COUNT];
    pthread_mutex_t mutex;
    pthread_cond_t cond;
    bool alive;
};

static struct msg_queue msg_queue[MODULE_ID_MAX] = { 0 };
static bool bus_initialized = false;

int msg_bus_init(void) {
    if(bus_initialized) {
        log_error("Message Bus already initialized\n");
        return -EAGAIN;
    }
    
    memset(msg_queue, 0, sizeof(msg_queue));
    for(int i = 0; i < MODULE_ID_MAX; i++) {
        msg_queue[i].head = 0;
        msg_queue[i].tail = 0;
        msg_queue[i].count = 0;
        msg_queue[i].alive = true;

        if(pthread_mutex_init(&msg_queue[i].mutex, NULL)) {
            log_error("Failed to initialize mutex for module %d\n", i);
            return -EFAULT;
        }
        if(pthread_cond_init(&msg_queue[i].cond, NULL)) {
            log_error("Failed to initialize condition for module %d\n", i);
            pthread_mutex_destroy(&msg_queue[i].mutex);
            return -EFAULT;
        }
    }
    bus_initialized = true;
    
    log_info("Message Bus initialized successfully\n");
    return SUCCESS;
}

void msg_bus_destroy(void) {
    if(!bus_initialized) {
        log_error("Message Bus not initialized\n");
        return ;
    }
    for(int i = 0; i < MODULE_ID_MAX; i++) {
        pthread_mutex_destroy(&msg_queue[i].mutex);
        pthread_cond_destroy(&msg_queue[i].cond);
    }
    bus_initialized = false;

    return ;
}

int send_message(const struct process_message *msg_in) {
    if(NULL == msg_in || msg_in->dst <= MODULE_ID_NONE || msg_in->dst >= MODULE_ID_MAX) {
        log_error("Invalid sender\n");
        return -EINVAL;
    }

    int dst = msg_in->dst;
    pthread_mutex_lock(&msg_queue[dst].mutex);
    if(msg_queue[dst].count >= MODULE_MAXMSG_COUNT) {
        log_error("Destination module %d queue full\n", dst);
        pthread_mutex_unlock(&msg_queue[dst].mutex);
        return -EOVERFLOW;
    }

    if(!msg_queue[dst].alive) {
        log_error("Destination module %d not alive\n", dst);
        pthread_mutex_unlock(&msg_queue[dst].mutex);
        return -ENOTCONN;
    }

    memcpy(&msg_queue[dst].messages[msg_queue[dst].tail], msg_in, sizeof(struct process_message));
    msg_queue[dst].tail = (msg_queue[dst].tail + 1) % MODULE_MAXMSG_COUNT;
    msg_queue[dst].count++;
    pthread_cond_signal(&msg_queue[dst].cond);
    pthread_mutex_unlock(&msg_queue[dst].mutex);

    return SUCCESS;
}

static inline void make_abs_timeout(struct timespec *ts, int timeout_ms) {
    clock_gettime(CLOCK_REALTIME, ts);
    ts->tv_sec += timeout_ms / 1000;
    ts->tv_nsec += timeout_ms % 1000 * 1000000;

    return ;
}

int receive_message(enum module_id self, struct process_message *msg_out, int timeout_ms) {
    int ret = SUCCESS;
    
    if(NULL == msg_out || self <= MODULE_ID_NONE || self >= MODULE_ID_MAX) {
        log_error("Invalid receiver\n");
        return -EINVAL;
    }

    pthread_mutex_lock(&msg_queue[self].mutex);
    if(timeout_ms <= 0) {
        while(msg_queue[self].count == 0 && msg_queue[self].alive) {
            pthread_cond_wait(&msg_queue[self].cond, &msg_queue[self].mutex);
        }
    }
    else {
        struct timespec ts;
        make_abs_timeout(&ts, timeout_ms);
        while(msg_queue[self].count == 0 && msg_queue[self].alive) {
            ret = pthread_cond_timedwait(&msg_queue[self].cond, &msg_queue[self].mutex, &ts);
        }
    }

    if(SUCCESS == ret && msg_queue[self].count > 0) {
        memcpy(msg_out, &msg_queue[self].messages[msg_queue[self].head], sizeof(struct process_message));
        msg_queue[self].head = (msg_queue[self].head + 1) % MODULE_MAXMSG_COUNT;
        msg_queue[self].count--;
        pthread_cond_signal(&msg_queue[self].cond);
    }
    pthread_mutex_unlock(&msg_queue[self].mutex);

    return ret;
}
