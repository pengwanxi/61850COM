/**
 *   \file glog.h
 *   \brief 全局日志
 */
#ifndef _GLOG_H_
#define _GLOG_H_

#include "log.h"
#include "config.h"
#include <libgen.h>
#include "iec61850_macro.h"

#define LOG_CONF_PATH IEC61850_CONFIG_PATH"/log"
#define LOG_CLOG_PATH IEC61850_CONFIG_PATH"/log/clog.json"

#ifndef LOGCFGPATH
#define LOGCFGPATH LOG_CONF_PATH
#else
#undef LOGCFGPATH
#define LOGCFGPATH LOG_CONF_PATH
#endif

typedef enum _LOG_INDEX {
    LOG_INDEX_0 = 0,
    GLOG, /* 通用 */
    STARTUP, /* 启动 */
    CFGLOG, /* 初始化文件 */
    COMMLOG, /* 通讯相关 */
    OUTLOG, /* 对外通讯 */
    PROLOG, /* 协议日志 */
    DEVLOG, /* 装置日志 */
    DVALOG, /* 装置数据日志 */
    MSGLOG, /* 消息日志 */
    SVCLOG /* 服务日志 */
} LOG_INDEX;

void log_conf_set_enable(int enable);
int log_conf_enable(void);

#define LOG_CONF_LEVEL_FRAME(c, l, name, buf, len, ...)                        \
    if (log_conf_enable()) {                                                   \
        log_name_frame(c, l, basename(__FILE__), __LINE__, name, buf, len,     \
                       __VA_ARGS__);                                           \
    }                                                                          \
    else {                                                                     \
        log_frame(c, l, basename(__FILE__), __LINE__, buf, len, __VA_ARGS__);  \
    }

#define LOG_CONF_LEVEL_BUF(c, l, name, ...)                                    \
    if (log_conf_enable()) {                                                   \
        log_name_buf(c, l, basename(__FILE__), __LINE__, name, __VA_ARGS__);   \
    }                                                                          \
    else {                                                                     \
        log_buf(c, l, __VA_ARGS__);                                            \
    }

/* 兼容以前 */
#define log_debug_frame(c, prefix, buf, len)                                   \
    log_frame(c, LOG_DEBUG, __FILE__, __LINE__, buf, len, "%s", prefix)

#define DEFAULT_PRINT_LEVEL (LOG_DEBUG)


/* 配置日志 */
#define CONF_LOG_DEBUG(name, ...)                                              \
    LOG_CONF_LEVEL_BUF(GLOG, LOG_DEBUG, name, __VA_ARGS__);

#define CONF_LOG_INFO(name, ...)                                               \
    LOG_CONF_LEVEL_BUF(GLOG, LOG_INFO, name, __VA_ARGS__);

#define CONF_LOG_WARN(name, ...)                                               \
    LOG_CONF_LEVEL_BUF(GLOG, LOG_WARN, name, __VA_ARGS__);

/* 协议日志 */
#define PRO_LOG_DEBUG_FRAME(name, buf, len, ...)                               \
    LOG_CONF_LEVEL_FRAME(PROLOG, LOG_DEBUG, name, buf, len, __VA_ARGS__);

#define PRO_LOG_INFO_FRAME(name, buf, len, ...)                                \
    LOG_CONF_LEVEL_FRAME(PROLOG, LOG_INFO, name, buf, len, __VA_ARGS__);

#define PRO_LOG_DEBUG_BUF(name, ...)                                           \
    LOG_CONF_LEVEL_BUF(PROLOG, LOG_DEBUG, name, __VA_ARGS__);

#define PRO_LOG_INFO_BUF(name, ...)                                            \
    LOG_CONF_LEVEL_BUF(PROLOG, LOG_INFO, name, __VA_ARGS__);

#define PRO_LOG_WARN_BUF(name, ...)                                            \
    LOG_CONF_LEVEL_BUF(PROLOG, LOG_WARN, name, __VA_ARGS__);

/* 通讯日志 */
#define PORT_LOG_DEBUG_FRAME(name, buf, len, ...)                              \
    LOG_CONF_LEVEL_FRAME(COMMLOG, LOG_DEBUG, name, buf, len, __VA_ARGS__);

#define PORT_LOG_DEBUG_BUF(name, ...)                                          \
    LOG_CONF_LEVEL_BUF(COMMLOG, LOG_DEBUG, name, __VA_ARGS__);

#define PORT_LOG_WARN_BUF(name, ...)                                           \
    LOG_CONF_LEVEL_BUF(COMMLOG, LOG_WARN, name, __VA_ARGS__);

/* 装置日志 */
#define DEV_LOG_DEBUG_BUF(name, ...)                                           \
    LOG_CONF_LEVEL_BUF(DEVLOG, LOG_DEBUG, name, __VA_ARGS__);

#define DEV_LOG_INFO_BUF(name, ...)                                            \
    LOG_CONF_LEVEL_BUF(DEVLOG, LOG_INFO, name, __VA_ARGS__);

#define DEV_LOG_WARN_BUF(name, ...)                                            \
    LOG_CONF_LEVEL_BUF(DEVLOG, LOG_WARN, name, __VA_ARGS__);

/* 装置数据日志 */
#define OUT_LOG_DEBUG_BUF(name, ...)                                          \
    LOG_CONF_LEVEL_BUF(OUTLOG, LOG_DEBUG, name, __VA_ARGS__);

#define OUT_LOG_INFO_BUF(name, ...)                                           \
    LOG_CONF_LEVEL_BUF(OUTLOG, LOG_INFO, name, __VA_ARGS__);

#define OUT_LOG_NOTICE_BUF(name, ...)                                         \
    LOG_CONF_LEVEL_BUF(OUTLOG, LOG_NOTICE, name, __VA_ARGS__);

#define OUT_LOG_WARN_BUF(name, ...)                                           \
    LOG_CONF_LEVEL_BUF(OUTLOG, LOG_WARN, name, __VA_ARGS__);

/* 装置数据日志 */
#define DVAL_LOG_DEBUG_BUF(name, ...)                                          \
    LOG_CONF_LEVEL_BUF(DVALOG, LOG_DEBUG, name, __VA_ARGS__);

#define DVAL_LOG_INFO_BUF(name, ...)                                           \
    LOG_CONF_LEVEL_BUF(DVALOG, LOG_INFO, name, __VA_ARGS__);

#define DVAL_LOG_NOTICE_BUF(name, ...)                                         \
    LOG_CONF_LEVEL_BUF(DVALOG, LOG_NOTICE, name, __VA_ARGS__);

#define DVAL_LOG_WARN_BUF(name, ...)                                           \
    LOG_CONF_LEVEL_BUF(DVALOG, LOG_WARN, name, __VA_ARGS__);

/* 数据库 */
#define DB_LOG_DEBUG_BUF(name, ...)                                            \
    LOG_CONF_LEVEL_BUF(DBLOG, LOG_DEBUG, name, __VA_ARGS__);

#define DB_LOG_INFO_BUF(name, ...)                                             \
    LOG_CONF_LEVEL_BUF(DBLOG, LOG_INFO, name, __VA_ARGS__);

#define DB_LOG_NOTICE_BUF(name, ...)                                           \
    LOG_CONF_LEVEL_BUF(DBLOG, LOG_NOTICE, name, __VA_ARGS__);

#define DB_LOG_WARN_BUF(name, ...)                                             \
    LOG_CONF_LEVEL_BUF(DBLOG, LOG_WARN, name, __VA_ARGS__);

int log_conf_init();

#endif /* _GLOG_H_ */
