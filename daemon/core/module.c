#include "core/module.h"
#include "common/log.h"
#include "core/msg_queue.h"

#include <unistd.h>

static inline int match_handler(const struct module *mod, const struct message *msg) {
    int ret = -ENOENT;
    int i = 0;
    while(NULL != mod->msg_handlers[i].handler) {
        if(mod->msg_handlers[i].type == msg->type) {    
            ret = i;
            break;
        }
        i++;
    }
    return ret;
}

static void *module_thread_func(void *arg) {
    struct module *mod = (struct module *)arg;
    if(NULL == mod || NULL == mod->msg_handlers) {
        log_error("Module is Invalid\n");
        return NULL;
    }

    int inner_ret = SUCCESS;
    if(NULL != mod->init) {
        inner_ret = mod->init(mod);
        if(SUCCESS != inner_ret) {
            log_error("Module [%s] init failed with ret %d\n", mod->name, inner_ret);
            mod->running = false;
            return NULL;
        }
    }

    struct message msg = { 0 };
    while(mod->running) {
        memset(&msg, 0, sizeof(struct message));
        inner_ret = receive_message(mod->id, &msg);
        if(SUCCESS == inner_ret) {
            int handler_index = match_handler(mod, &msg);
            if(-ENOENT != handler_index) {
                inner_ret = mod->msg_handlers[handler_index].handler(mod, &msg);
                log_info("Module [%s] handle message %d ret %d\n", mod->name, msg.type, inner_ret);
            }
        }
        else if(-ENOTCONN == inner_ret) {
            log_error("Message Queue is not ready, module [%s] is stopped\n", mod->name);
            mod->running = false;
        }
    }

    return NULL;
}

int module_init(struct module *mod, enum module_id id, const char *name) {
    if (NULL == mod || NULL == name) {
        log_error("Invalid arguments\n");
        return -EINVAL;
    }

    mod->running = false;
    mod->id = id;
    mod->msg_handlers = NULL;
    snprintf(mod->name, MODULE_NAME_MAX_LEN, "%s", name);
    mod->thread_id = 0;

    log_info("Module [%s] initialized with ID %d\n", mod->name, mod->id);
    return SUCCESS;
}

int module_start(struct module *mod) {
    if (NULL == mod) {
        log_error("Module is Invalid\n");
        return -EINVAL;
    }

    log_info("Module [%s] started\n", mod->name);
    mod->running = true;

    if(pthread_create(&mod->thread_id, NULL, module_thread_func, mod)) {
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

    log_info("Module [%s] stopping due to reason: %s\n", mod->name, reason ? reason : "No reason provided");

    mod->running = false;
    if(pthread_join(mod->thread_id, NULL)) {
        log_error("Failed to join thread for module [%s]\n", mod->name);
        return -EBUSY;
    }

    log_info("Module [%s] stopped successfully\n", mod->name);
    return SUCCESS;
}
