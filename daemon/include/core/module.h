#ifndef __MODULE_H__
#define __MODULE_H__

#include "modules/module_ids.h"
#include "core/msg_queue.h"
#include <pthread.h>
#include <stdbool.h>

#define MODULE_NAME_MAX_LEN 32

struct module;

struct msg_handler {
    uint32_t event_type;
    int (*handler)(struct module *mod, const struct message *msg);
};

struct module {
    volatile bool running;
    enum module_id id;
    char name[MODULE_NAME_MAX_LEN];
    pthread_t thread_id;

    const struct msg_handler *msg_handlers;
    int (*init)(struct module *mod);
    int (*exit)(struct module *mod);
};

int module_init(struct module *mod, enum module_id id, const char *name);
int module_start(struct module *mod);
int module_stop(struct module *mod, const char *reason);

int alive_check_handler(struct module *mod, const struct message *msg);

#endif