#include "core/module_registry.h"
#include "log.h"
#include "modules/module_initcalls.h"

#include <signal.h>
#include <unistd.h>

void signal_handler(int sig);

static volatile sig_atomic_t g_should_exit = 0;

static module_register_fn g_module_register_table[] = {
    memory_module_register,
};

int main() {
    int ret = module_registry_init();
    if (SUCCESS != ret) {
        return ret;
    }

    signal(SIGTERM, signal_handler);
    signal(SIGINT, signal_handler);

    for (unsigned int i = 0; i < sizeof(g_module_register_table) / sizeof(g_module_register_table[0]); ++i) {
        ret = g_module_register_table[i]();
        if (SUCCESS != ret) {
            return ret;
        }
    }

    print_module_registry();

    ret = module_registry_start_all();
    if (SUCCESS != ret) {
        return ret;
    }

    while(!g_should_exit) {
        pause();
    }

    return module_registry_stop_all("signal");
}

void signal_handler(int sig) {
    g_should_exit = 1;
    log_info("receive signal %d, exit this daemon\n", sig);

    return ;
}
