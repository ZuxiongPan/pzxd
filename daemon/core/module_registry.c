#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <stdbool.h>
#include "datastructure.h"
#include "module_registry.h"
#include "log.h"

static armd_list_t registry_sentinel[MAX_LEVEL];
static bool registry_initialized = false;

void registry_init(void)
{
    if (registry_initialized)
    {
        return ;
    }

    for (int i = 0; i < MAX_LEVEL; i++)
    {
        armd_list_init(&registry_sentinel[i]);
    }
    registry_initialized = true;

    return ;
}

void armd_module_register(armd_module_t *mod, armd_module_level_e level)
{
    if (mod == NULL || level >= MAX_LEVEL || !registry_initialized)
    {
        return ;
    }

    armd_list_init(&mod->list);
    armd_list_insert(&registry_sentinel[level], &mod->list);

    log_info("module '%s' registered at level %d", mod->name, (int)level);

    return ;
}

void armd_module_unregister(armd_module_t *mod, armd_module_level_e level)
{
    if (mod == NULL || level >= MAX_LEVEL || !registry_initialized)
    {
        return ;
    }

    armd_list_delete(&mod->list);

    log_info("module '%s' unregistered from level %d", mod->name, (int)level);

    return ;
}

armd_module_t* armd_module_find(const char *name)
{
    if (name == NULL || !registry_initialized)
    {
        return NULL;
    }

    for (int level = SYSTEM_LEVEL; level < MAX_LEVEL; level++)
    {
        armd_list_t *sentinel = &registry_sentinel[level];
        armd_list_t *node = sentinel->next;

        while (node != sentinel)
        {
            armd_module_t *mod = member_container(node, armd_module_t, list);
            if (strncmp(name, mod->name, MODULE_NAME_MAXLEN) == 0)
            {
                return mod;
            }
            node = node->next;
        }
    }

    return NULL;
}

int armd_module_startall(uv_loop_t *loop)
{
    if (loop == NULL || !registry_initialized)
    {
        return -EINVAL;
    }

    for (int level = SYSTEM_LEVEL; level < MAX_LEVEL; level++)
    {
        armd_list_t *sentinel = &registry_sentinel[level];
        armd_list_t *node = sentinel->next;

        while (node != sentinel)
        {
            armd_module_t *mod = member_container(node, armd_module_t, list);
            int ret = armd_module_start(mod, loop);
            if (ret < 0)
            {
                log_error("failed to start module '%s' (ret=%d)", mod->name, ret);
                return ret;
            }
            node = node->next;
        }
    }

    return 0;
}

void armd_module_stopall(void)
{
    if (!registry_initialized)
    {
        return ;
    }

    for (int level = MAX_LEVEL - 1; level >= SYSTEM_LEVEL; level--)
    {
        armd_list_t *sentinel = &registry_sentinel[level];

        while (!armd_list_empty(sentinel))
        {
            armd_list_t *node = sentinel->next;
            armd_module_t *mod  = member_container(node, armd_module_t, list);

            armd_module_stop(mod);
            armd_list_delete(node);
        }
    }

    return ;
}
