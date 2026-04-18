#ifndef __MODULE_REGISTRY_H__
#define __MODULE_REGISTRY_H__

#include "modules/module_ids.h"
#include "module.h"

typedef int (*module_register_fn)(void);

int module_registry_init(void);
int module_register(struct module *mod);
int module_registry_start_all(void);
int module_registry_stop_all(const char *reason);
struct module *find_module_by_id(enum module_id id);
struct module *find_module_by_name(const char *name);

void print_module_registry(void);

#endif
