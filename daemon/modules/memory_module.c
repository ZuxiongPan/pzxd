#include "core/module.h"
#include "core/module_registry.h"
#include "common/log.h"

static struct module memory_module = { 0 };

static const struct module_actions memory_module_actions = {
    .on_init = default_module_on_init,
    .on_message = default_module_on_message,
    .on_tick = default_module_on_tick,
    .on_exit = default_module_on_exit
};

int memory_module_register(void) {
    int ret = module_init(&memory_module, MODULE_ID_MEMORY, "Memory-Module");
    if (SUCCESS != ret) {
        log_error("Failed to initialize Memory Module with error code %d\n", ret);
        return ret;
    }

    memory_module.actions = &memory_module_actions;

    ret = module_register(&memory_module);
    if (SUCCESS != ret) {
        return ret;
    }

    log_info("Memory Module registered successfully\n");
    return SUCCESS;
}
