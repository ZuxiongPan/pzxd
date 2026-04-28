#include "modules/module_initcalls.h"
#include "modules/module_ids.h"

#include <stddef.h>

module_register_fn module_register_table[] = {
    [MODULE_ID_TEST1] = test1_module_register,
    [MODULE_ID_TEST2] = test2_module_register,
    [MODULE_ID_MAX] = NULL,
};

const size_t module_register_table_size = sizeof(module_register_table) / sizeof(module_register_table[0]);
