#include "common/comm_conf.h"
#include "common/log.h"
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
    if (!fp)
        return -ENOENT;

    fseek(fp, 0, SEEK_END);
    long size = ftell(fp);
    rewind(fp);

    char *buf = malloc(size + 1);
    if (!buf) {
        fclose(fp);
        return -ENOMEM;
    }

    fread(buf, 1, size, fp);
    buf[size] = '\0';
    fclose(fp);

    cJSON *root = cJSON_Parse(buf);
    free(buf);

    if (!root || !cJSON_IsObject(root))
        return -EINVAL;

    pthread_rwlock_wrlock(&db.rwlock);

    if (db.root)
        cJSON_Delete(db.root);

    db.root = root;

    pthread_rwlock_unlock(&db.rwlock);

    return 0;
}


int database_save(void)
{
    char tmpfile[FILENAME_MAXLEN];
    snprintf(tmpfile, sizeof(tmpfile), "%s.tmp", CONFIG_FILE);

    pthread_rwlock_rdlock(&db.rwlock);

    if (!db.root) {
        pthread_rwlock_unlock(&db.rwlock);
        return -EINVAL;
    }

    char *json = cJSON_PrintUnformatted(db.root);
    pthread_rwlock_unlock(&db.rwlock);

    if (!json)
        return -ENOMEM;

    FILE *fp = fopen(tmpfile, "w");
    if (!fp) {
        free(json);
        return -ENOENT;
    }

    fwrite(json, 1, strlen(json), fp);
    fflush(fp);
    fsync(fileno(fp));
    fclose(fp);

    free(json);

    return rename(tmpfile, CONFIG_FILE);
}

int database_init(void) {
    db.root = NULL;
    if(pthread_rwlock_init(&db.rwlock, NULL) != 0) {
        log_error("Failed to initialize rwlock\n");
        return -EPERM;
    }
    
    return database_load();
}

void database_destroy(void) {
    database_save();

    pthread_rwlock_wrlock(&db.rwlock);
    if(NULL != db.root) {
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

    while (*p && cur) {
        const char *dot = strchr(p, '.');
        int len = dot ? (dot - p) : strlen(p);

        if (len >= (int)sizeof(key))
            return NULL;

        strncpy(key, p, len);
        key[len] = '\0';

        cur = cJSON_GetObjectItemCaseSensitive(cur, key);
        if (!cur)
            return NULL;

        if (!dot)
            break;

        p = dot + 1;
    }

    return cur;
}

static cJSON* db_create_path(const char *path)
{
    char key[KEYNAME_MAXLEN];
    const char *p = path;
    cJSON *cur = db.root;

    while (*p) {
        const char *dot = strchr(p, '.');
        int len = dot ? (dot - p) : strlen(p);

        strncpy(key, p, len);
        key[len] = '\0';

        cJSON *next = cJSON_GetObjectItemCaseSensitive(cur, key);

        if (!next) {
            next = cJSON_CreateObject();
            cJSON_AddItemToObject(cur, key, next);
        }

        cur = next;

        if (!dot)
            break;

        p = dot + 1;
    }

    return cur;
}

int database_get(const char *path, enum datatype type, void *value, int size)
{
    if (!path || !value)
        return -EINVAL;

    pthread_rwlock_rdlock(&db.rwlock);

    cJSON *node = db_get_node(path);
    if (!node) {
        pthread_rwlock_unlock(&db.rwlock);
        return -ENOENT;
    }

    int ret = 0;

    switch (type) {
    case DBTYPE_INT:
        if (cJSON_IsNumber(node))
            *(int *)value = node->valueint;
        else
            ret = -EINVAL;
        break;

    case DBTYPE_BOOL:
        if (cJSON_IsBool(node))
            *(bool *)value = cJSON_IsTrue(node);
        else
            ret = -EINVAL;
        break;

    case DBTYPE_STRING:
        if (cJSON_IsString(node)) {
            strncpy((char *)value, node->valuestring, size - 1);
            ((char *)value)[size - 1] = '\0';
        } else {
            ret = -EINVAL;
        }
        break;

    default:
        ret = -EINVAL;
    }

    pthread_rwlock_unlock(&db.rwlock);
    return ret;
}

int database_set(const char *path, enum datatype type, const void *value)
{
    if (!path || !value)
        return -EINVAL;

    pthread_rwlock_wrlock(&db.rwlock);

    cJSON *node = db_get_node(path);

    if (!node) {
        node = db_create_path(path);
        if (!node) {
            pthread_rwlock_unlock(&db.rwlock);
            return -ENOMEM;
        }

        char *last_dot = strrchr(path, '.');
        const char *key = last_dot ? last_dot + 1 : path;

        cJSON_DeleteItemFromObjectCaseSensitive(
            last_dot ? db_get_node(path) : db.root, key);
    }

    cJSON *new_item = NULL;

    switch (type) {
    case DBTYPE_INT:
        new_item = cJSON_CreateNumber(*(int *)value);
        break;

    case DBTYPE_BOOL:
        new_item = cJSON_CreateBool(*(bool *)value);
        break;

    case DBTYPE_STRING:
        new_item = cJSON_CreateString((char *)value);
        break;

    default:
        pthread_rwlock_unlock(&db.rwlock);
        return -EINVAL;
    }

    if (!new_item) {
        pthread_rwlock_unlock(&db.rwlock);
        return -ENOMEM;
    }

    char *last_dot = strrchr(path, '.');
    const char *key = last_dot ? last_dot + 1 : path;

    cJSON *parent = last_dot ? db_get_node(strndup(path, last_dot - path)) : db.root;

    if (!parent) {
        pthread_rwlock_unlock(&db.rwlock);
        return -ENOENT;
    }

    cJSON_ReplaceItemInObjectCaseSensitive(parent, key, new_item);

    pthread_rwlock_unlock(&db.rwlock);

    return SUCCESS;
}