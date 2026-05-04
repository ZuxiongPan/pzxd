#include "core/module_registry.h"
#include "modules/module_initcalls.h"
#include "common/comm_def.h"
#include "core/msg_queue.h"
#include "core/database.h"

#include <signal.h>
#include <unistd.h>

void signal_handler(int sig);

extern module_register_fn module_register_table[];
extern const size_t module_register_table_size;

static volatile sig_atomic_t g_should_exit = 0;

int main() 
{
    int ret = SUCCESS;

    signal(SIGTERM, signal_handler);
    signal(SIGINT, signal_handler);

    ret = database_init();
    if (SUCCESS != ret) 
    {
        log_error("database_init failed, ret: %d\n", ret);
        return ret;
    }

    ret = msg_queue_init();
    if (SUCCESS != ret) 
    {
        log_error("msg_queue_init failed, ret: %d\n", ret);
        database_destroy();
        return ret;
    }

    module_registry_init();

    for (size_t i = 0; i < module_register_table_size; ++i) 
    {
        if(NULL == module_register_table[i]) 
        {
            continue;
        }
        ret = module_register_table[i]();
        if (SUCCESS != ret) 
        {
            msg_queue_destroy();
            database_destroy();
            return ret;
        }
    }

    ret = module_registry_start_all();
    if (SUCCESS != ret)
    {
        msg_queue_destroy();
        module_registry_stop_all("start error");
        database_destroy();
        return ret;
    }

    while(!g_should_exit) 
    {
    }

    msg_queue_destroy();
    module_registry_stop_all("signal");
    database_destroy();

    return SUCCESS;
}

void signal_handler(int sig) 
{
    g_should_exit = 1;
    log_info("receive signal %d, exit this daemon\n", sig);

    return ;
}
