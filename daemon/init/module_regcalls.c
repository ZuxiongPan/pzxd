#include "module_registry.h"
#include "module_regcalls.h"

const armd_module_regcall_t armd_module_regcalls_system[] = {
    armd_monitor_register,
    NULL,
};

const armd_module_regcall_t armd_module_regcalls_core[] = {
    NULL,
};

const armd_module_regcall_t armd_module_regcalls_app[] = {
    NULL,
};

const armd_module_regcall_t armd_module_regcalls_post[] = {
    NULL,
};
