#ifndef __MEMORY_MODULE_H__
#define __MEMORY_MODULE_H__

int memory_module_register(void);

typedef int (*module_register_fn)(void);

#endif
