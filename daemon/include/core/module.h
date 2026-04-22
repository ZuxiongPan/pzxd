#ifndef __MODULE_H__
#define __MODULE_H__

#include "modules/module_ids.h"
#include <pthread.h>
#include <stdbool.h>

#define MODULE_NAME_MAX_LEN 32

struct module;

struct module_actions {
    int (*on_init)(struct module *mod);
    int (*on_message)(struct module *mod);
    int (*on_tick)(struct module *mod);
    int (*on_exit)(struct module *mod);
};

struct module {
    enum module_id id;
    char name[MODULE_NAME_MAX_LEN];

    bool running;
    pthread_t thread_id;

    const struct module_actions *actions;
};

int module_init(struct module *mod, enum module_id id, const char *name);
int module_start(struct module *mod);
int module_stop(struct module *mod, const char *reason);
int module_pass_message(struct module *mod, void *msg);

int default_module_on_init(struct module *mod);
int default_module_on_exit(struct module *mod);
int default_module_on_message(struct module *mod);
int default_module_on_tick(struct module *mod);

#endif