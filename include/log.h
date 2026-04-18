#ifndef __LOG_H__
#define __LOG_H__

#include "inner_errno.h"
#include <stdio.h>

#define log_error(fmt, ...) \
    printf("[%s]@%d-ERROR# " fmt, __FILE__, __LINE__, ##__VA_ARGS__)
#define log_info(fmt, ...) \
    printf("[%s]@%d-INFO# " fmt, __FILE__, __LINE__, ##__VA_ARGS__)
#define log_debug(fmt, ...) \
    printf("[%s]@%d-DEBUG# " fmt, __FILE__, __LINE__, ##__VA_ARGS__)

#endif