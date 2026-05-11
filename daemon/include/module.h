#pragma once

#include <uv.h>
#include <pthread.h>
#include <stdbool.h>
#include "message.h"
#include "datastructure.h"

#define QUEUE_SIZE 64
#define MAX_MODULE_NUMS 32
#define MODULE_NAME_MAXLEN 32

struct module;

typedef struct armd_module_operations {
    int (*on_start)(struct module*);
    int (*on_message)(struct module*, const armd_msg_t*);
    int (*on_stop)(struct module*);
} armd_module_ops_t;

typedef struct armd_msg_queue {
    armd_msg_t msgs[QUEUE_SIZE];
    unsigned int head;
    unsigned int tail;
    pthread_mutex_t q_mutex;
} armd_msg_queue_t;

typedef struct module {
    char name[MODULE_NAME_MAXLEN];

    uv_loop_t *loop;
    uv_async_t async;
    armd_msg_queue_t msg_queue;
    armd_list_t list;
    void *priv; // module private data

    const armd_module_ops_t *mops;

} armd_module_t;

int armd_module_start(armd_module_t *mod, uv_loop_t *loop);
int armd_module_post(armd_module_t *mod, const armd_msg_t *msg);
void armd_module_stop(armd_module_t *mod);
