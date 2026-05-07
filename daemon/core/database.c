#include "common/comm_conf.h"
#include "common/comm_def.h"
#include "core/database.h"
#include "cJSON/cJSON.h"

#include <pthread.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <unistd.h>

struct database {
    cJSON *root;
    pthread_rwlock_t rwlock;
};

static struct database db = { 0 };

static int database_load(void)
{
    FILE *fp = fopen(CONFIG_FILE, "r");
    if (NULL == fp)
    {
        log_error("Failed to open config file %s\n", CONFIG_FILE);
        return -ENOENT;
    }

    fseek(fp, 0, SEEK_END);
    long size = ftell(fp);
    rewind(fp);

    char *buf = malloc(size + 1);
    if (NULL == buf)
    {
        log_error("Failed to allocate memory for config file %s\n", CONFIG_FILE);
        fclose(fp);
        return -ENOMEM;
    }

    fread(buf, 1, size, fp);
    buf[size] = '\0';
    fclose(fp);

    cJSON *root = cJSON_Parse(buf);
    free(buf);

    if (NULL == root || !cJSON_IsObject(root))
    {
        log_error("Invalid config file %s\n", CONFIG_FILE);
        return -EINVAL;
    }

    pthread_rwlock_wrlock(&db.rwlock);
    if (NULL != db.root)
    {
        log_info("old config exists, delete it\n");
        cJSON_Delete(db.root);
    }

    db.root = root;
    pthread_rwlock_unlock(&db.rwlock);

    return SUCCESS;
}

int database_save(void)
{
    char tmpfile[FILENAME_MAXLEN];
    snprintf(tmpfile, sizeof(tmpfile), "%s.tmp", CONFIG_FILE);

    pthread_rwlock_rdlock(&db.rwlock);

    if (NULL == db.root)
    {
        log_error("config is empty\n");
        pthread_rwlock_unlock(&db.rwlock);
        return -EINVAL;
    }

    char *json = cJSON_Print(db.root);
    pthread_rwlock_unlock(&db.rwlock);

    if (NULL == json)
    {
        log_error("Failed to print config to JSON\n");
        return -ENOMEM;
    }

    FILE *fp = fopen(tmpfile, "w");
    if (NULL == fp)
    {
        log_error("Failed to open tmp file %s\n", tmpfile);
        free(json);
        return -ENOENT;
    }

    fwrite(json, 1, strlen(json), fp);
    free(json);
    fflush(fp);
    fsync(fileno(fp));
    fclose(fp);

    return rename(tmpfile, CONFIG_FILE);
}

int database_init(void)
{
    db.root = NULL;
    if (0 != pthread_rwlock_init(&db.rwlock, NULL))
    {
        log_error("Failed to initialize rwlock\n");
        return -EPERM;
    }
    
    return database_load();
}

void database_destroy(void)
{
    database_save();

    pthread_rwlock_wrlock(&db.rwlock);
    if (NULL != db.root)
    {
        cJSON_Delete(db.root);
    }
    pthread_rwlock_unlock(&db.rwlock);
    pthread_rwlock_destroy(&db.rwlock);

    return ;
}

static cJSON* db_get_node(const char *path)
{
    char key[KEYNAME_MAXLEN];
    const char *p = path;
    cJSON *cur = db.root;

    while (*p && cur)
    {
        const char *dot = strchr(p, '.');
        int len = dot ? (dot - p) : strlen(p);

        if (len >= (int)sizeof(key))
        {
            return NULL;
        }

        strncpy(key, p, len);
        key[len] = '\0';

        cur = cJSON_GetObjectItemCaseSensitive(cur, key);
        if (NULL == cur)
        {
            return NULL;
        }

        if (NULL == dot)
        {
            break;
        }

        p = dot + 1;
    }

    return cur;
}

static cJSON* db_create_path(const char *path)
{
    char key[KEYNAME_MAXLEN];
    const char *p = path;
    cJSON *cur = db.root;

    while (*p)
    {
        const char *dot = strchr(p, '.');
        int len = dot ? (dot - p) : strlen(p);

        strncpy(key, p, len);
        key[len] = '\0';

        cJSON *next = cJSON_GetObjectItemCaseSensitive(cur, key);

        if (NULL == next)
        {
            next = cJSON_CreateObject();
            cJSON_AddItemToObject(cur, key, next);
        }

        cur = next;

        if (NULL == dot)
        {
            break;
        }

        p = dot + 1;
    }

    return cur;
}

int database_get(const char *path, enum datatype type, void *value, int size)
{
    if (NULL == path || NULL == value)
    {
        return -EINVAL;
    }

    pthread_rwlock_rdlock(&db.rwlock);

    cJSON *node = db_get_node(path);
    if (NULL == node)
    {
        pthread_rwlock_unlock(&db.rwlock);
        return -ENOENT;
    }

    int ret = -EINVAL;

    switch (type)
    {
        case DBTYPE_BOOL:
            if (cJSON_IsBool(node))
            {
                *(bool *)value = cJSON_IsTrue(node);
                ret = SUCCESS;
            }
            break;

        case DBTYPE_INT:
            if (cJSON_IsNumber(node))
            {
                *(int *)value = node->valueint;
                ret = SUCCESS;
            }
            break;

        case DBTYPE_STRING:
            if (cJSON_IsString(node))
            {
                strncpy((char *)value, node->valuestring, size - 1);
                ((char *)value)[size - 1] = '\0';
                ret = SUCCESS;
            }
            break;

        default:
            ret = -EINVAL;
            break;
    }

    pthread_rwlock_unlock(&db.rwlock);
    return ret;
}

int database_set(const char *path, enum datatype type, const void *value)
{
    if (NULL == path || NULL == value)
    {
        log_error("path or value is NULL\n");
        return -EINVAL;
    }

    pthread_rwlock_wrlock(&db.rwlock);

    cJSON *node = db_get_node(path);
    if (NULL == node)
    {
        node = db_create_path(path);
        if (NULL == node)
        {
            log_error("Failed to create path %s\n", path);
            pthread_rwlock_unlock(&db.rwlock);
            return -ENOMEM;
        }

        char *last_dot = strrchr(path, '.');
        const char *key = last_dot ? last_dot + 1 : path;

        cJSON_DeleteItemFromObjectCaseSensitive(
            last_dot ? db_get_node(path) : db.root, key);
    }

    cJSON *new_item = NULL;

    switch (type)
    {
        case DBTYPE_BOOL:
            new_item = cJSON_CreateBool(*(bool *)value);
            break;
        
        case DBTYPE_INT:
            new_item = cJSON_CreateNumber(*(int *)value);
            break;

        case DBTYPE_STRING:
            new_item = cJSON_CreateString((char *)value);
            break;

        default:
            pthread_rwlock_unlock(&db.rwlock);
            return -EINVAL;
    }

    if (NULL == new_item)
    {
        pthread_rwlock_unlock(&db.rwlock);
        return -ENOMEM;
    }

    char *last_dot = strrchr(path, '.');
    const char *key = last_dot ? last_dot + 1 : path;

    char *tmp_path = last_dot ? strndup(path, last_dot - path) : NULL;
    cJSON *parent = last_dot ? db_get_node(tmp_path) : db.root;

    if(NULL != tmp_path)
    {
        free(tmp_path);
    }

    if (NULL == parent)
    {
        pthread_rwlock_unlock(&db.rwlock);
        return -ENOENT;
    }

    cJSON_ReplaceItemInObjectCaseSensitive(parent, key, new_item);

    pthread_rwlock_unlock(&db.rwlock);

    return SUCCESS;
}