#pragma once

#include <stddef.h>

#define member_container(member_ptr, container_type, member_name) \
    ((container_type *)((char *)(member_ptr) - offsetof(container_type, member_name)))

typedef struct armd_double_linked_list {
    struct armd_double_linked_list *prev;
    struct armd_double_linked_list *next;
} armd_list_t;

static inline void armd_list_init(armd_list_t *list)
{
    list->prev = list;
    list->next = list;
}

static inline void armd_list_insert(armd_list_t *list, armd_list_t *node)
{
    list->prev->next = node;
    node->prev       = list->prev;
    list->prev       = node;
    node->next       = list;
}

static inline void armd_list_delete(armd_list_t *node)
{
    node->prev->next = node->next;
    node->next->prev = node->prev;
    node->prev = node;
    node->next = node;
}

static inline int armd_list_empty(const armd_list_t *list)
{
    return list->next == list;
}