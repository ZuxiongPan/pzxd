#include "modules/module_ids.h"
#include "modules/module_initcalls.h"
#include "core/module.h"
#include "core/module_registry.h"
#include "core/msg_queue.h"
#include "common/log.h"
#include "core/database.h"

static struct module test1_module = { 0 };

static int msg_handler(struct module *mod, const struct message *msg) {
    log_info("Received message of type %d, content: %s\n", msg->type, MSG_PAYLOAD(msg, char));
    
    char buf[STRVALUE_MAXLEN] = { 0 };
    int ret = database_get("app.name", DBTYPE_STRING, buf, sizeof(buf));
    ERR_CHECK_RET(ret);
    log_info("app.name: %s\n", buf);
    ret = database_get("app.type", DBTYPE_STRING, buf, sizeof(buf));
    ERR_CHECK_RET(ret);
    log_info("app.type: %s\n", buf);
    ret = database_get("app.platform", DBTYPE_STRING, buf, sizeof(buf));
    ERR_CHECK_RET(ret);
    log_info("app.platform: %s\n", buf);
    ret = database_get("app.version", DBTYPE_STRING, buf, sizeof(buf));
    ERR_CHECK_RET(ret);
    log_info("app.version: %s\n", buf);
    ret = database_get("app.language", DBTYPE_STRING, buf, sizeof(buf));
    ERR_CHECK_RET(ret);
    log_info("app.language: %s\n", buf);
    ret = database_get("app.description", DBTYPE_STRING, buf, sizeof(buf));
    ERR_CHECK_RET(ret);
    log_info("app.description: %s\n", buf);
    ret = database_get("app.author.name", DBTYPE_STRING, buf, sizeof(buf));
    ERR_CHECK_RET(ret);
    log_info("app.author.name: %s\n", buf);
    ret = database_get("app.author.email", DBTYPE_STRING, buf, sizeof(buf));
    ERR_CHECK_RET(ret);
    log_info("app.author.email: %s\n", buf);
    ret = database_get("app.license", DBTYPE_STRING, buf, sizeof(buf));
    ERR_CHECK_RET(ret);
    log_info("app.license: %s\n", buf);
    ret = database_get("app.release_date", DBTYPE_STRING, buf, sizeof(buf));
    ERR_CHECK_RET(ret);
    log_info("app.release_date: %s\n", buf);

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
