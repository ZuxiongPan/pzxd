#pragma once

#include "module.h"

typedef enum armd_module_level {
    SYSTEM_LEVEL = 0,
    CORE_LEVEL,
    APP_LEVEL,
    POST_LEVEL,
    MAX_LEVEL
} armd_module_level_e;

void armd_module_register(armd_module_t *mod, armd_module_level_e level);
void armd_module_unregister(armd_module_t *mod, armd_module_level_e level);
int armd_module_startall(uv_loop_t *loop);
void armd_module_stopall(void);
void registry_init(void);
armd_module_t* armd_module_find(const char *name);
