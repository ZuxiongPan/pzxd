#include "modules/module_initcalls.h"

#include <stddef.h>

module_register_fn module_register_table[] = {
    memory_module_register,
};

const size_t module_register_table_size = sizeof(module_register_table) / sizeof(module_register_table[0]);
