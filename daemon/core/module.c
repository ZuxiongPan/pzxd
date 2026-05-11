#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <stdbool.h>
#include "module.h"
#include "log.h"

static void async_callback(uv_async_t *handle)
{
    armd_module_t *mod = (armd_module_t *)handle->data;
    armd_msg_queue_t *q = &mod->msg_queue;

    while (true)
    {
        pthread_mutex_lock(&q->q_mutex);

        if (q->tail == q->head)
        {
            pthread_mutex_unlock(&q->q_mutex);
            break;
        }

        armd_msg_t msg = q->msgs[q->tail];
        q->tail = (q->tail + 1) % QUEUE_SIZE;

        pthread_mutex_unlock(&q->q_mutex);

        if (mod->mops != NULL && mod->mops->on_message != NULL)
        {
            mod->mops->on_message(mod, &msg);
        }
    }
}

int armd_module_start(armd_module_t *mod, uv_loop_t *loop)
{
    if (mod == NULL || loop == NULL || mod->mops == NULL)
    {
        return -EINVAL;
    }

    mod->loop = loop;

    armd_msg_queue_t *q = &mod->msg_queue;
    pthread_mutex_init(&q->q_mutex, NULL);
    q->head = 0;
    q->tail = 0;

    int ret = uv_async_init(loop, &mod->async, async_callback);
    if (ret < 0)
    {
        log_error("uv_async_init failed for module '%s': %s",
                  mod->name, uv_strerror(ret));
        pthread_mutex_destroy(&q->q_mutex);
        return ret;
    }
    mod->async.data = mod;

    if (mod->mops->on_start != NULL)
    {
        ret = mod->mops->on_start(mod);
        if (ret < 0)
        {
            log_error("on_start failed for module '%s': %d", mod->name, ret);
            uv_close((uv_handle_t *)&mod->async, NULL);
            pthread_mutex_destroy(&q->q_mutex);
            return ret;
        }
    }

    log_info("module '%s' started", mod->name);
    return 0;
}

void armd_module_stop(armd_module_t *mod)
{
    if (mod == NULL || mod->mops == NULL)
    {
        return;
    }

    if (mod->mops->on_stop != NULL)
    {
        mod->mops->on_stop(mod);
    }

    uv_close((uv_handle_t *)&mod->async, NULL);
    pthread_mutex_destroy(&mod->msg_queue.q_mutex);

    log_info("module '%s' stopped", mod->name);
    return ;
}

int armd_module_post(armd_module_t *mod, const armd_msg_t *msg)
{
    if (mod == NULL || msg == NULL)
    {
        return -EINVAL;
    }

    armd_msg_queue_t *q = &mod->msg_queue;

    pthread_mutex_lock(&q->q_mutex);

    unsigned int next_head = (q->head + 1) % QUEUE_SIZE;
    if (next_head == q->tail)
    {
        pthread_mutex_unlock(&q->q_mutex);
        log_warn("module '%s' queue full, message dropped", mod->name);
        return -ENOSPC;
    }

    q->msgs[q->head] = *msg;
    q->head = next_head;

    pthread_mutex_unlock(&q->q_mutex);

    uv_async_send(&mod->async);

    return 0;
}
