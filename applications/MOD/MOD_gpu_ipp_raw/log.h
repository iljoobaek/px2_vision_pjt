#ifndef __LOG_H__
#define __LOG_H__

#include <stdio.h>

typedef enum {
    LOG_ERR,
    LOG_MSG,
    LOG_PERF,
    LOG_INFO,
    LOG_DBG,
} LOG_T;

#define log_printf(log, fmt, ...) \
    do {    \
        if (log <= log_level) {  \
            printf(fmt, ##__VA_ARGS__); \
        }   \
    } while (0)


#define log_printf_verbose(log, fmt, ...)   \
    do {    \
        if (log <= log_level) {  \
            printf("[%s:%d] " fmt "\n", __func__, __LINE__, ##__VA_ARGS__);  \
        } \
    } while (0)

#define ERR(fmt, ...) \
    log_printf_verbose(LOG_ERR, fmt, ##__VA_ARGS__)

#define MSG(fmt, ...)   \
    log_printf(LOG_MSG, fmt, ##__VA_ARGS__)

#define PERF(fmt, ...)  \
    log_printf(LOG_PERF, fmt, ##__VA_ARGS__)

#define INFO(fmt, ...) \
    log_printf(LOG_INFO, fmt, ##__VA_ARGS__)

#define DBG(fmt, ...)   \
    log_printf_verbose(LOG_DBG, fmt, ##__VA_ARGS__)

#define ENTER()   \
    DBG("enter")

#define LEAVE() \
    DBG("leave")

#endif
