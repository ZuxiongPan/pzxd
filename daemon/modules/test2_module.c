#include "modules/module_ids.h"
#include "modules/module_initcalls.h"
#include "core/module.h"
#include "core/module_registry.h"
#include "core/msg_queue.h"
#include "core/database.h"
#include "common/comm_def.h"
#include "common/comm_event.h"

static struct module test2_module = { 0 };

static int msg_handler(struct module *mod, const struct message *msg) 
{
    log_info("Received message of event %x, content: %s\n", msg->event, MSG_PAYLOAD(msg, char));
    
    char buf[STRVALUE_MAXLEN] = { 0 };
    time_t t = time(NULL);
    snprintf(buf, sizeof(buf), "%s", ctime(&t));
    int ret = database_set("app.release_date", DBTYPE_STRING, buf);
    ERR_CHECK_RET(ret);

    return SUCCESS;
}

static int test2_module_init(struct module *mod) 
{
    log_info("test2_module_init\n");
    struct message msg = { 0 };
    msg.src = mod->id;
    msg.dst = MODULE_ID_TEST1;
    msg.event = EV_TEST_EVENT1;
    msg.payload_size = snprintf(msg.payload, sizeof(msg.payload), "test2 to test1");
    send_message(&msg);

    return SUCCESS;
}

const struct msg_handler test2_msg_handlers[] = {
    { EV_ALIVE_CHECK, alive_check_handler },
    { EV_TEST_EVENT1, msg_handler },
    { INVALID_EVENT_TYPE, NULL }
};

int test2_module_register(void) 
{
    log_info("test2_module_register\n");
    memset(&test2_module, 0, sizeof(test2_module));
    test2_module.id = MODULE_ID_TEST2;
    snprintf(test2_module.name, sizeof(test2_module.name), "test2_module");
    test2_module.msg_handlers = test2_msg_handlers;
    test2_module.init = test2_module_init;
    
    return module_register(&test2_module);
}
