#pragma once

#include <stdio.h>

#define log_info(fmt, ...) \
    do { printf("[%s:%u]INFO# " fmt "\n", __FILE__, __LINE__, ##__VA_ARGS__); } while (0)

#define log_warn(fmt, ...) \
    do { printf("[%s:%u]WARN# " fmt "\n", __FILE__, __LINE__, ##__VA_ARGS__); } while (0)

#define log_error(fmt, ...) \
    do { printf("[%s:%u]ERROR# " fmt "\n", __FILE__, __LINE__, ##__VA_ARGS__); } while (0)
