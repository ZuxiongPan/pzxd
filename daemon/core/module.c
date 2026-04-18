#include "core/module.h"
#include "log.h"

#include <unistd.h>

static void *module_thread_func(void *arg) {
    struct module *mod = (struct module *)arg;
    if(NULL == mod) {
        log_error("Module is Invalid\n");
        return ERRNO_TO_PTR(INVALID_ARGUMENT);
    }

    if(NULL == mod->actions) {
        log_info("Module [%s] has no actions defined, exiting thread\n", mod->name);
        return ERRNO_TO_PTR(OPERATION_FORBIDDEN);
    }

    int inner_ret = SUCCESS;

    if(NULL != mod->actions->on_init) {
        inner_ret = mod->actions->on_init(mod);
        if(SUCCESS != inner_ret) {
            log_error("Module [%s] on_init failed with error code %d\n", mod->name, inner_ret);
            return ERRNO_TO_PTR(inner_ret);
        }
    }

    while(mod->running) {
        if(NULL != mod->actions->on_message) {
            inner_ret = mod->actions->on_message(mod);
            log_debug("Module [%s] on_message returned with code %d\n", mod->name, inner_ret);
        }

        if(NULL != mod->actions->on_tick) {
            inner_ret = mod->actions->on_tick(mod);
        }

        for (int i = 0; i < 30 && mod->running; ++i) {
            sleep(1);
        }
    }

    if(NULL != mod->actions->on_exit) {
        inner_ret = mod->actions->on_exit(mod);
    }

    return ERRNO_TO_PTR(SUCCESS);
}

int module_init(struct module *mod, enum module_id id, const char *name) {
    if (NULL == mod || NULL == name) {
        log_error("Invalid arguments\n");
        return INVALID_ARGUMENT;
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
        return INVALID_ARGUMENT;
    }

    mod->running = true;
    log_info("Module [%s] started\n", mod->name);

    if(0 != pthread_create(&mod->thread_id, NULL, module_thread_func, mod)) {
        log_error("Failed to create thread for module [%s]\n", mod->name);
        mod->running = false;
        return CFUNC_CALLFAIL;
    }

    return SUCCESS;
}

int module_stop(struct module *mod, const char *reason) {
    if (NULL == mod) {
        log_error("Module is Invalid\n");
        return INVALID_ARGUMENT;
    }

    mod->running = false;
    log_info("Module [%s] stopping due to reason: %s\n", mod->name, reason ? reason : "No reason provided");

    if(0 != pthread_join(mod->thread_id, NULL)) {
        log_error("Failed to join thread for module [%s]\n", mod->name);
        return CFUNC_CALLFAIL;
    }

    log_info("Module [%s] stopped successfully\n", mod->name);
    return SUCCESS;
}

int module_pass_message(struct module *mod, void *msg) {
    if (NULL == mod) {
        log_error("Module is Invalid\n");
        return INVALID_ARGUMENT;
    }

    log_debug("Module [%s] received a message\n", mod->name);

    return SUCCESS;
}
