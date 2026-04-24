#include "core/module.h"
#include "common/log.h"
#include "core/msg_bus.h"

#include <unistd.h>

static void *module_thread_func(void *arg) {
    struct module *mod = (struct module *)arg;
    if(NULL == mod) {
        log_error("Module is Invalid\n");
        return NULL;
    }

    if(NULL == mod->actions) {
        log_info("Module [%s] has no actions defined, exiting thread\n", mod->name);
        return NULL;
    }

    int inner_ret = SUCCESS;

    if(NULL != mod->actions->on_init) {
        inner_ret = mod->actions->on_init(mod);
        if(SUCCESS != inner_ret) {
            log_error("Module [%s] on_init failed with error code %d\n", mod->name, inner_ret);
            return NULL;
        }
    }

    while(mod->running) {
        struct process_message msg;
        int ret = receive_message(mod->id, &msg, 5000);
        if(SUCCESS == ret && NULL != mod->actions->on_message) {
            inner_ret = mod->actions->on_message(mod, &msg);
            log_debug("Module [%s] on_message returned with code %d\n", mod->name, inner_ret);
        }
        else if(ETIMEDOUT == ret && NULL != mod->actions->on_tick) {
            inner_ret = mod->actions->on_tick(mod);
        }
    }

    if(NULL != mod->actions->on_exit) {
        inner_ret = mod->actions->on_exit(mod);
    }

    return NULL;
}

int module_init(struct module *mod, enum module_id id, const char *name) {
    if (NULL == mod || NULL == name) {
        log_error("Invalid arguments\n");
        return -EINVAL;
    }

    mod->id = id;
    snprintf(mod->name, MODULE_NAME_MAX_LEN, "%s", name);
    mod->running = false;
    mod->thread_id = 0;

    log_info("Module [%s] initialized with ID %d\n", mod->name, mod->id);
    return SUCCESS;
}

int module_start(struct module *mod) {
    if (NULL == mod) {
        log_error("Module is Invalid\n");
        return -EINVAL;
    }

    mod->running = true;
    log_info("Module [%s] started\n", mod->name);

    if(0 != pthread_create(&mod->thread_id, NULL, module_thread_func, mod)) {
        log_error("Failed to create thread for module [%s]\n", mod->name);
        mod->running = false;
        return -EBUSY;
    }

    return SUCCESS;
}

int module_stop(struct module *mod, const char *reason) {
    if (NULL == mod) {
        log_error("Module is Invalid\n");
        return -EINVAL;
    }

    mod->running = false;
    log_info("Module [%s] stopping due to reason: %s\n", mod->name, reason ? reason : "No reason provided");

    if(pthread_join(mod->thread_id, NULL)) {
        log_error("Failed to join thread for module [%s]\n", mod->name);
        return -EBUSY;
    }

    log_info("Module [%s] stopped successfully\n", mod->name);
    return SUCCESS;
}

int default_module_on_init(struct module *mod) {
    if(NULL != mod) {
        log_info("Module [%s], ID %d, default init\n", mod->name, mod->id);
    }

    return SUCCESS;
}

int default_module_on_message(struct module *mod) {
    if(NULL != mod) {
        log_info("Module [%s], ID %d, default on_message\n", mod->name, mod->id);
    }

    return SUCCESS;
}

int default_module_on_tick(struct module *mod) {    
    if(NULL != mod) {
        log_info("Module [%s], ID %d, default on_tick\n", mod->name, mod->id);
    }

    return SUCCESS;
}

int default_module_on_exit(struct module *mod) {
    if(NULL != mod) {
        log_info("Module [%s], ID %d, default on_exit\n", mod->name, mod->id);
    }

    return SUCCESS;
}
