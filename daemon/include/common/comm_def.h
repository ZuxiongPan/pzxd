#ifndef __COMM_DEF_H__
#define __COMM_DEF_H__

#include <errno.h>
#include <stdio.h>

#define log_error(fmt, ...) \
    printf("[%s]@%d-ERROR# " fmt, __FILE__, __LINE__, ##__VA_ARGS__)
#define log_info(fmt, ...) \
    printf("[%s]@%d-INFO# " fmt, __FILE__, __LINE__, ##__VA_ARGS__)
#ifdef DAEMON_DEBUG
#define log_debug(fmt, ...) \
    printf("[%s]@%d-DEBUG# " fmt, __FILE__, __LINE__, ##__VA_ARGS__)
#else
#define log_debug(fmt, ...) 
#endif

#define SUCCESS 0
// error codes using kernel errno

#define ERR_CHECK_RET(ret) \
    do { \
        if (SUCCESS != ret) { \
            return ret; \
        } \
    } while (0)

#endif