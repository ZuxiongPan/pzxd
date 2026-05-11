#include <signal.h>
#include <stdlib.h>
#include "module.h"
#include "module_registry.h"
#include "module_regcalls.h"
#include "log.h"

static uv_async_t g_shutdown_async;

static void on_shutdown_async(uv_async_t *handle)
{
    log_info("shutdown signal received, stopping all modules …");
    armd_module_stopall();
    uv_close((uv_handle_t *)handle, NULL);
}

static void signal_handler(int signum)
{
    (void)signum;
    uv_async_send(&g_shutdown_async);
}

static int run_regcalls(const armd_module_regcall_t *calls, const char *level_name)
{
    if (calls == NULL) return 0;

    for (int i = 0; calls[i] != NULL; i++)
    {
        int ret = calls[i]();
        if (ret < 0)
        {
            log_error("regcall[%d] at level '%s' failed: %d", i, level_name, ret);
            return ret;
        }
    }
    return 0;
}

int main(void)
{
    int ret = 0;
    uv_loop_t *loop = uv_default_loop();

    ret = uv_async_init(loop, &g_shutdown_async, on_shutdown_async);
    if (ret < 0)
    {
        log_error("uv_async_init for shutdown failed: %s", uv_strerror(ret));
        return 1;
    }

    signal(SIGINT, signal_handler);
    signal(SIGTERM, signal_handler);

    registry_init();
    
    if ((ret = run_regcalls(armd_module_regcalls_system, "system")) < 0)
    {
        log_error("run_regcalls for system failed: %d", ret);
        goto fail;
    }
    if ((ret = run_regcalls(armd_module_regcalls_core, "core")) < 0)
    {
        log_error("run_regcalls for core failed: %d", ret);
        goto fail;
    }
    if ((ret = run_regcalls(armd_module_regcalls_app, "app")) < 0)
    {
        log_error("run_regcalls for app failed: %d", ret);
        goto fail;
    }
    if ((ret = run_regcalls(armd_module_regcalls_post, "post")) < 0)
    {
        log_error("run_regcalls for post failed: %d", ret);
        goto fail;
    }

    ret = armd_module_startall(loop);
    if (ret < 0)
    {
        log_error("armd_module_startall failed: %d", ret);
        goto fail;
    }

    log_info("armd daemon running");
    uv_run(loop, UV_RUN_DEFAULT);
    log_info("armd daemon exited cleanly");

    uv_loop_close(loop);
    
    return 0;

fail:
    armd_module_stopall();
    uv_close((uv_handle_t *)&g_shutdown_async, NULL);
    uv_run(loop, UV_RUN_DEFAULT);
    uv_loop_close(loop);

    return 1;
}
