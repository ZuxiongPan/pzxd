#pragma once

typedef int (*armd_module_regcall_t)(void);

extern const armd_module_regcall_t armd_module_regcalls_system[];
extern const armd_module_regcall_t armd_module_regcalls_core[];
extern const armd_module_regcall_t armd_module_regcalls_app[];
extern const armd_module_regcall_t armd_module_regcalls_post[];

extern int armd_monitor_register(void);