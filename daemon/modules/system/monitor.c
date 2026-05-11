#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include "module.h"
#include "module_registry.h"
#include "events.h"
#include "log.h"

#define MONITOR_NAME "monitor"
#define MONITOR_INTERVAL_MS 5000u

typedef struct {
    uv_timer_t  timer;
    uint64_t    tick;
} monitor_priv_t;

static int  monitor_on_start  (armd_module_t *mod);
static int  monitor_on_message(armd_module_t *mod, const armd_msg_t *msg);
static int  monitor_on_stop   (armd_module_t *mod);

static const armd_module_ops_t monitor_ops = {
    .on_start   = monitor_on_start,
    .on_message = monitor_on_message,
    .on_stop    = monitor_on_stop,
};

static monitor_priv_t g_monitor_priv;

static armd_module_t g_monitor_module = {
    .name  = MONITOR_NAME,
    .loop  = NULL,
    .priv  = &g_monitor_priv,
    .mops  = &monitor_ops,
};

/* ------------------------------------------------------------------ *
 * Timer callback — fires in the event-loop thread                    *
 * ------------------------------------------------------------------ */
static void monitor_timer_cb(uv_timer_t *timer)
{
    armd_module_t  *mod  = (armd_module_t *)timer->data;
    monitor_priv_t *priv = (monitor_priv_t *)mod->priv;

    priv->tick++;
    log_info("[monitor] heartbeat tick #%llu", (unsigned long long)priv->tick);

    armd_msg_t msg;
    set_message(&msg,
                MONITOR_NAME,       /* src  */
                MONITOR_NAME,       /* dst  */
                MSG_DIR_INNER,
                EV_MONITOR_ALIVE,
                (const void *)&priv->tick,
                (uint32_t)sizeof(priv->tick));

    int ret = armd_module_post(mod, &msg);
    if (ret < 0)
    {
        log_warn("[monitor] armd_module_post failed: %d", ret);
    }
}

static int monitor_on_start(armd_module_t *mod)
{
    monitor_priv_t *priv = (monitor_priv_t *)mod->priv;
    priv->tick = 0;

    int ret = uv_timer_init(mod->loop, &priv->timer);
    if (ret < 0)
    {
        log_error("[monitor] uv_timer_init: %s", uv_strerror(ret));
        return ret;
    }

    priv->timer.data = mod;   /* back-pointer for the callback */

    ret = uv_timer_start(&priv->timer, monitor_timer_cb,
                         MONITOR_INTERVAL_MS, MONITOR_INTERVAL_MS);
    if (ret < 0)
    {
        log_error("[monitor] uv_timer_start: %s", uv_strerror(ret));
        return ret;
    }

    log_info("[monitor] started, heartbeat every %u ms", MONITOR_INTERVAL_MS);
    return 0;
}

static int monitor_on_message(armd_module_t *mod, const armd_msg_t *msg)
{
    (void)mod;

    switch (msg->event)
    {
        case EV_MONITOR_ALIVE:
        {
            uint64_t tick = 0;
            if (msg->payload_len >= sizeof(tick))
            {
                memcpy(&tick, msg->payload, sizeof(tick));
            }
            log_info("[monitor] received EV_MONITOR_ALIVE tick #%llu",
                     (unsigned long long)tick);
            break;
        }
        default:
            log_warn("[monitor] unknown event 0x%08x", msg->event);
            break;
    }

    return 0;
}

static int monitor_on_stop(armd_module_t *mod)
{
    monitor_priv_t *priv = (monitor_priv_t *)mod->priv;
    uv_timer_stop(&priv->timer);
    uv_close((uv_handle_t *)&priv->timer, NULL);
    log_info("[monitor] stopped");
    return 0;
}

int armd_monitor_register(void)
{
    armd_module_register(&g_monitor_module, SYSTEM_LEVEL);
    return 0;
}
