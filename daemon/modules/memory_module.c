#include "core/module.h"
#include "core/module_registry.h"
#include "log.h"

static struct module memory_module = { 0 };

static int memory_module_on_init(struct module *mod) {
    log_info("Initializing Memory Module\n");
    return SUCCESS;
}

static int memory_module_on_message(struct module *mod) {
    log_info("Memory Module received a message\n");
    return SUCCESS;
}

static int memory_module_on_tick(struct module *mod) {
    log_info("Memory Module tick\n");
    return SUCCESS;
}

static int memory_module_on_exit(struct module *mod) {
    log_info("Exiting Memory Module\n");
    return SUCCESS;
}

static const struct module_actions memory_module_actions = {
    .on_init = memory_module_on_init,
    .on_message = memory_module_on_message,
    .on_tick = memory_module_on_tick,
    .on_exit = memory_module_on_exit
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
