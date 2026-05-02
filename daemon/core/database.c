#include "common/comm_conf.h"
#include "common/log.h"
#include "core/database.h"
#include "cJSON/cJSON.h"

#include <pthread.h>
#include <stdlib.h>
#include <string.h>

struct database {
    cJSON *root;
    pthread_rwlock_t rwlock;
};

static struct database db = { 0 };

static int database_load(void) {
    cJSON *tmp = NULL;
    FILE *fp = fopen(CONFIG_FILE, "r");
    if(NULL == fp) {
        log_error("Failed to open config file: %s\n", CONFIG_FILE);
        return -ENOENT;
    }

    fseek(fp, 0, SEEK_END);
    long filesize = ftell(fp);
    rewind(fp);

    char *buffer = (char *)malloc(filesize + 1);
    if(NULL == buffer) {
        log_error("Failed to allocate memory for config file\n");
        fclose(fp);
        return -ENOMEM;
    }

    size_t read_size = fread(buffer, 1, filesize, fp);
    if(read_size != (size_t)filesize) {
        log_error("Failed to read config file\n");
        free(buffer);
        fclose(fp);
        return -EIO;
    }
    buffer[filesize] = '\0';
    fclose(fp);

    tmp = cJSON_Parse(buffer);
    free(buffer);

    if(NULL == tmp) {
        log_error("Failed to parse config file\n");
        return -EINVAL;
    }

    pthread_rwlock_wrlock(&db.rwlock);
    if(NULL != db.root) {
        cJSON_Delete(db.root);
    }
    db.root = tmp;
    pthread_rwlock_unlock(&db.rwlock);

    return SUCCESS;
}

static int database_save(void) {
    int ret = SUCCESS;

    pthread_rwlock_wrlock(&db.rwlock);
    if(NULL == db.root) {
        pthread_rwlock_unlock(&db.rwlock);
        return -EINVAL;
    }
    
    char *buffer = cJSON_Print(db.root);
    pthread_rwlock_unlock(&db.rwlock);
    if(NULL == buffer) {
        return -ENOMEM;
    }

    FILE *fp = fopen(CONFIG_FILE, "w");
    if(NULL == fp) {
        log_error("Failed to open config file for writing: %s\n", CONFIG_FILE);
        free(buffer);
        return -ENOENT;
    }

    size_t size = strlen(buffer);
    size_t write_size = fwrite(buffer, 1, size, fp);
    if(write_size != size) {
        log_error("Failed to write config file\n");
        ret = -EIO;
    }
    fclose(fp);
    free(buffer);

    return ret;
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
    int ret = database_save();
    if(SUCCESS != ret) {
        log_error("Failed to save database with error code %d\n", ret);
    }

    pthread_rwlock_wrlock(&db.rwlock);
    if(NULL != db.root) {
        cJSON_Delete(db.root);
    }
    pthread_rwlock_unlock(&db.rwlock);
    pthread_rwlock_destroy(&db.rwlock);
}