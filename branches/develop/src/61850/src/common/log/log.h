/**
 *   \file log.h
 *   \brief 日志功能
 *
 *  目前主要用zlog
 *
 */
#ifndef _LOG_H_
#define _LOG_H_

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "zlog.h"

#define LOGCFGPATH "./"

#define LOG_BACKUP
#ifdef LOG_BACKUP
#include "log_backup.h"
#endif

#define LOG_LIST_NUM (32)
#define LOG_SUB_ARRAY_NUM (32)

/*  */
typedef struct _LOG_LIST_UNIT_CLASS {
    char name[64];
    int level;
} LOG_LIST_UNIT_CLASS;

typedef struct _LOG_LIST_UNIT {
    int index;
    char name[32];
    void *fd;
    int cnum;
    LOG_LIST_UNIT_CLASS cname[LOG_SUB_ARRAY_NUM];
} LOG_LIST_UNIT;

/*  */
typedef struct _LOG_LIST {
    LOG_LIST_UNIT unit[LOG_LIST_NUM];
} LOG_LIST;

/*  */
typedef enum _LOG_LEVEL {
#ifndef LOG_DEBUG
    LOG_DEBUG = 0,
#endif
#ifndef LOG_INFO
    LOG_INFO = 1,
#endif
#ifndef LOG_NOTICE
    LOG_NOTICE = 2,
#endif
#ifndef LOG_WARN
    LOG_WARN = 3,
#endif
#ifndef LOG_ERROR
    LOG_ERROR = 4,
#endif
#ifndef LOG_FATAL
    LOG_FATAL = 5,
#endif
} LOG_LEVEL;

#define FRAME_LEN 512
#define PREFIX_LEN 512

void *log_list_fd(int index);

#define log_fatal(c, ...)                                                      \
    zlog_fatal((zlog_category_t *)log_list_fd(c), __VA_ARGS__)
#define log_error(c, ...)                                                      \
    zlog_error((zlog_category_t *)log_list_fd(c), __VA_ARGS__)
#define log_warn(c, ...)                                                       \
    zlog_warn((zlog_category_t *)log_list_fd(c), __VA_ARGS__)
#define log_notice(c, ...)                                                     \
    zlog_notice((zlog_category_t *)log_list_fd(c), __VA_ARGS__)
#define log_info(c, ...)                                                       \
    zlog_info((zlog_category_t *)log_list_fd(c), __VA_ARGS__)
#define log_debug(c, ...)                                                      \
    zlog_debug((zlog_category_t *)log_list_fd(c), __VA_ARGS__)

#define log_level_buf(c, l, ...)                                               \
    {                                                                          \
        switch (l) {                                                           \
        case LOG_DEBUG: {                                                      \
            log_debug(c, __VA_ARGS__);                                         \
        } break;                                                               \
        case LOG_INFO: {                                                       \
            log_info(c, __VA_ARGS__);                                          \
        } break;                                                               \
        case LOG_NOTICE: {                                                     \
            log_notice(c, __VA_ARGS__);                                        \
        } break;                                                               \
        case LOG_WARN: {                                                       \
            log_warn(c, __VA_ARGS__);                                          \
        } break;                                                               \
        case LOG_ERROR: {                                                      \
            log_error(c, __VA_ARGS__);                                         \
        } break;                                                               \
        case LOG_FATAL: {                                                      \
            log_fatal(c, __VA_ARGS__);                                         \
        } break;                                                               \
        default:                                                               \
            log_debug(c, __VA_ARGS__);                                         \
            break;                                                             \
        }                                                                      \
    }

void log_frame(int c, int level, char *file, int line, unsigned char *buf,
               int len, const char *fmt, ...);
void log_name_frame(int c, int level, char *file, int line, char *cname,
                    unsigned char *buf, int len, const char *fmt, ...);

#define log_buf(c, level, ...) log_level_buf(c, level, __VA_ARGS__)

void log_name_buf(int c, int level, char *file, int line, char *cname,
                  const char *fmt, ...);

/**
 *  \brief 日志初始化
 *  \param config
 *  \param name
 *  \return !NULL 成功
 *  \return NULL 失败
 */
void *log_init(const char *name);

/**
 *  \brief 日志列表初始化
 *  \param config
 *  \param name
 *  \return !NULL 成功
 *  \return NULL 失败
 */
void log_list_init(char *name, LOG_LIST list);

/**
 *  \brief 分类名称设置
 *  \param config
 *  \param name
 *  \return !NULL 成功
 *  \return NULL 失败
 */
int log_set_cname(int index, int num, LOG_LIST_UNIT_CLASS *cname);
int log_find_cname(int index, char *cname);

/**
 *  \brief 日志退出
 *  \param config
 *  \param name
 *  \return >0 成功
 *  \return <=0 失败
 */
void log_exit(void);

#include "log_conf.h"

#endif /* _LOG_H_ */
