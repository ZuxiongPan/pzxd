#ifndef __MODULE_H__
#define __MODULE_H__

#include "modules/module_ids.h"
#include "core/msg_queue.h"
#include <pthread.h>
#include <stdbool.h>

#define MODULE_NAME_MAX_LEN 32

struct module;

struct msg_handler {
    enum msg_type type;
    int (*handler)(struct module *mod, const struct message *msg);
};

struct module {
    bool running;
    enum module_id id;
    char name[MODULE_NAME_MAX_LEN];
    pthread_t thread_id;

    const struct msg_handler *msg_handlers;
};

int module_init(struct module *mod, enum module_id id, const char *name);
int module_start(struct module *mod);
int module_stop(struct module *mod, const char *reason);

#endif