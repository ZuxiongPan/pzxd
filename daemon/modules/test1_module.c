#include "modules/module_ids.h"
#include "modules/module_initcalls.h"
#include "core/module.h"
#include "core/module_registry.h"
#include "core/msg_queue.h"
#include "common/log.h"

static struct module test1_module = { 0 };

static int msg_handler(struct module *mod, const struct message *msg) {
    log_info("Received message of type %d, content: %s\n", msg->type, MSG_PAYLOAD(msg, char));
    
    time_t t = time(NULL);
    struct message res = { 0 };
    res.src = mod->id;
    res.dst = msg->src;
    res.type = MSG_TYPE_NONE;
    res.event = 0;
    res.payload_size = snprintf(res.payload, sizeof(res.payload), "%s", ctime(&t));
    send_message(&res);

    return SUCCESS;
}

const struct msg_handler test1_msg_handlers[] = {
    { MSG_TYPE_NONE, msg_handler },
    { MSG_TYPE_INVALID, NULL }
};

int test1_module_register(void) {
    memset(&test1_module, 0, sizeof(test1_module));
    test1_module.id = MODULE_ID_TEST1;
    snprintf(test1_module.name, sizeof(test1_module.name), "test1_module");
    test1_module.msg_handlers = test1_msg_handlers;
    test1_module.init = NULL;

    return module_register(&test1_module);
}
