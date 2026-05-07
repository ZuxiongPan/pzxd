#include "core/module_registry.h"
#include "common/comm_def.h"

#include <string.h>

static struct module *module_registry[MODULE_ID_MAX];

void module_registry_init(void)
{
    for (int i = 0; i < MODULE_ID_MAX; ++i)
    {
        module_registry[i] = NULL;
    }

    return ;
}

int module_register(struct module *mod)
{
    if (NULL == mod || mod->id >= MODULE_ID_MAX)
    {
        log_error("Invalid module\n");
        return -EINVAL;
    }

    if (NULL != module_registry[mod->id])
    {
        log_error("Module with id %d already registered\n", mod->id);
        return -EEXIST;
    }

    module_registry[mod->id] = mod;
    return SUCCESS;
}

int module_registry_start_all(void)
{
    for (int i = 0; i < MODULE_ID_MAX; ++i)
    {
        if (NULL == module_registry[i])
        {
            continue;
        }

        int ret = module_start(module_registry[i]);
        if (SUCCESS != ret)
        {
            log_error("Failed to start module [%s] with error code %d\n",
                      module_registry[i]->name, ret);
            return -EBUSY;
        }
    }

    return SUCCESS;
}

int module_registry_stop_all(const char *reason)
{
    for (int i = MODULE_ID_MAX - 1; i >= 0; --i)
    {
        if (NULL == module_registry[i] || 0 == module_registry[i]->thread_id)
        {
            continue;
        }

        int ret = module_stop(module_registry[i], reason);
        if (SUCCESS != ret)
        {
            log_error("Failed to stop module [%s] with error code %d\n",
                      module_registry[i]->name, ret);
            // a module stop failed do not influence other modules
        }
    }

    return SUCCESS;
}

struct module *find_module_by_id(enum module_id id)
{
    if (id <= MODULE_ID_NONE || id >= MODULE_ID_MAX)
    {
        log_error("Invalid module id %d\n", id);
        return NULL;
    }

    return module_registry[id];
}

struct module *find_module_by_name(const char *name)
{
    struct module *entry = NULL;

    if (NULL == name)
    {
        log_error("Invalid module name\n");
        return NULL;
    }
    for (int i = 0; i < MODULE_ID_MAX; ++i)
    {
        if (NULL != module_registry[i] && !strncmp(module_registry[i]->name, name, MODULE_NAME_MAX_LEN))
        {
            entry = module_registry[i];
            break;
        }
    }

    return entry;
}

void print_module_registry(void)
{
    for (int i = 0; i < MODULE_ID_MAX; ++i)
    {
        if (NULL != module_registry[i])
        {
            printf("\tModule %d: %s\n", i, module_registry[i]->name);
        }
    }
}
