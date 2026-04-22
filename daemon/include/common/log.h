#ifndef __LOG_H__
#define __LOG_H__

#include <errno.h>
#include <stdio.h>

#define log_error(fmt, ...) \
    printf("[%s]@%d-ERROR# " fmt, __FILE__, __LINE__, ##__VA_ARGS__)
#define log_info(fmt, ...) \
    printf("[%s]@%d-INFO# " fmt, __FILE__, __LINE__, ##__VA_ARGS__)
#define log_debug(fmt, ...) \
    printf("[%s]@%d-DEBUG# " fmt, __FILE__, __LINE__, ##__VA_ARGS__)

#define SUCCESS 0
// error codes using kernel errno

#endif