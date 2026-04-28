#ifndef __MODULE_INITCALLS_H__
#define __MODULE_INITCALLS_H__

int test1_module_register(void);
int test2_module_register(void);

typedef int (*module_register_fn)(void);

#endif
