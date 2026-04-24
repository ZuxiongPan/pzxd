#include "core/module_registry.h"
#include "modules/module_initcalls.h"
#include "common/log.h"
#include "core/msg_bus.h"

#include <signal.h>
#include <unistd.h>

void signal_handler(int sig);

extern module_register_fn module_register_table[];
extern const size_t module_register_table_size;

static volatile sig_atomic_t g_should_exit = 0;

int main() {

    int ret = SUCCESS;

    signal(SIGTERM, signal_handler);
    signal(SIGINT, signal_handler);

    ret = msg_bus_init();
    if (SUCCESS != ret) {
        return ret;
    }

    module_registry_init();

    for (size_t i = 0; i < module_register_table_size; ++i) {
        ret = module_register_table[i]();
        if (SUCCESS != ret) {
            return ret;
        }
    }

    ret = module_registry_start_all();
    if (SUCCESS != ret) {
        return ret;
    }

    while(!g_should_exit) {
        time_t now = time(NULL);
        char buffer[128];
        struct process_message msg;
        for(int i = 0; i < 30; ++i) {
            now = time(NULL);
            snprintf(buffer, sizeof(buffer), "tick %d %s", i, ctime(&now));
            set_message(&msg, MODULE_ID_NONE, MODULE_ID_MEMORY, MSG_TYPE_NONE, \
                sizeof(buffer), buffer);
            send_message(&msg);
            sleep(5);
        }
    }

    module_registry_stop_all("signal");
    msg_bus_destroy();

    return SUCCESS;
}

void signal_handler(int sig) {
    g_should_exit = 1;
    module_registry_stop_all("signal");
    msg_bus_destroy();
    log_info("receive signal %d, exit this daemon\n", sig);

    return ;
}
