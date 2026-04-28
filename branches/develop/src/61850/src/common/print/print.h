/**
 *   \file print.h
 *   \brief 打印配置
 */
#ifndef _PRINT_H_
#define _PRINT_H_
#include <libgen.h>
#include <stdio.h>

/*  */
typedef enum _PRINT_LEVEL {
    PRINT_LEVEL_DEBUG = 0,
    PRINT_LEVEL_NOTICE,
    PRINT_LEVEL_INFO,
    PRINT_LEVEL_WARN,
    PRINT_LEVEL_ERROR,
    PRINT_LEVEL_FATAL,
} PRINT_LEVEL;

#define PRINT_RET() printf("\n")
#define DEFAULT_PRINT_LEVEL (PRINT_LEVEL_INFO)

#define PRINT_BUF(L, ...)                                                      \
    if (DEFAULT_PRINT_LEVEL <= PRINT_LEVEL_##L) {                              \
        printf("%s %s:%lu:%s:", #L, basename(__FILE__), __LINE__,              \
               __FUNCTION__);                                                  \
        printf(__VA_ARGS__);                                                   \
        PRINT_RET();                                                           \
    }

static inline void PRINT_HEX(unsigned char *p, int len)
{
    int i;
    for (i = 0; i < len; i++) {
        printf(" %02x", (unsigned char)p[i]);
    }
    printf("\n");
}

#define PRINT_FRAME(L, p, len, ...)                                            \
    if (DEFAULT_PRINT_LEVEL <= PRINT_LEVEL_##L) {                              \
        printf("%s %s:%d:%s:", #L, basename(__FILE__), __LINE__,               \
               __FUNCTION__);                                                  \
        printf(__VA_ARGS__);                                                   \
        PRINT_HEX(p, (int)len);                                                \
    }

/* 打印 */
#define PRINT_DEBUG(...) PRINT_BUF(DEBUG, __VA_ARGS__)
#define PRINT_NOTICE(...) PRINT_BUF(NOTICE, __VA_ARGS__)
#define PRINT_INFO(...) PRINT_BUF(INFO, __VA_ARGS__)
#define PRINT_WARN(...) PRINT_BUF(WARN, __VA_ARGS__)
#define PRINT_ERROR(...) PRINT_BUF(ERROR, __VA_ARGS__)
#define PRINT_FATAL(...) PRINT_BUF(FATAL, __VA_ARGS__)

#define PRINT_DEBUG_FRAME(p, l, ...) PRINT_FRAME(DEBUG, p, l, __VA_ARGS__)
#define PRINT_INFO_FRAME(p, l, ...) PRINT_FRAME(INFO, p, l, __VA_ARGS__)

#endif /* _PRINT_H_ */
