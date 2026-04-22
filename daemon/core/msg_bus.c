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
    bool mod_alive;
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
        msg_queue[i].mod_alive = false;

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
