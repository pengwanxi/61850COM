/**
 *   \file cjson_def.h
 *   \brief 方便调用的一些关于cJSON 的宏
 */
#ifndef _CJSON_DEF_H_
#define _CJSON_DEF_H_

#include "cJSON.h"
#include "log.h"

#define cjson_def_get_str_err(c, dest, json, key)                              \
    {                                                                          \
        cJSON *p = cJSON_GetObjectItemCaseSensitive(json, #key);               \
        if (!cJSON_IsString(p) || NULL == p->valuestring) {                    \
            log_warn(c, "%s null or not string", #key);                        \
            return -1;                                                         \
        }                                                                      \
        dest = strdup(p->valuestring);                                         \
        if (dest == NULL) {                                                   \
            log_error(c, "%s strdup failed", #key);                           \
            return -1;                                                         \
        }                                                                      \
        log_debug(c, "%s=%s ", #key, dest);                                  \
    }

#define cjson_def_get_str_default(c, dest, json, key, val)                     \
    {                                                                          \
        cJSON *p = cJSON_GetObjectItemCaseSensitive(json, #key);               \
        if (!cJSON_IsString(p) || NULL == p->valuestring) {                    \
            log_debug(c, "%s null or not string default %s", #key, val);       \
            if (NULL != val) {                                                 \
                dest = strdup(val);                                            \
                if (dest == NULL) {                                            \
                    log_error(c, "%s strdup failed", #key);                   \
                    return -1;                                                 \
                }                                                               \
            }                                                                  \
            else {                                                             \
                dest = NULL;                                                   \
            }                                                                  \
        }                                                                      \
        else {                                                                 \
            dest = strdup(p->valuestring);                                     \
            if (dest == NULL) {                                                \
                log_error(c, "%s strdup failed", #key);                       \
                return -1;                                                     \
            }                                                                  \
        }                                                                      \
        log_debug(c, "%s=%s", #key, dest);                                    \
    }

#define cjson_def_get_ival_err(c, dest, json, key)                             \
    {                                                                          \
        cJSON *p = cJSON_GetObjectItemCaseSensitive(json, #key);               \
        if (!cJSON_IsNumber(p) || NULL == p) {                                 \
            log_warn(c, "%s null or not number", #key);                        \
            return -1;                                                         \
        }                                                                      \
        log_debug(c, "%s=%d", #key, p->valueint);                              \
        dest = p->valueint;                                                    \
    }

#define cjson_def_get_ival_default(c, dest, json, key, val)                    \
    {                                                                          \
        cJSON *p = cJSON_GetObjectItemCaseSensitive(json, #key);               \
        if (!cJSON_IsNumber(p) || NULL == p) {                                 \
            log_debug(c, "%s null(%p) or not number", #key, p);                \
            dest = val;                                                        \
        }                                                                      \
        else {                                                                 \
            dest = p->valueint;                                                \
        }                                                                      \
        log_debug(c, "%s=%d", #key, dest);                                     \
    }

#define cjson_def_get_fval_err(c, dest, json, key)                             \
    {                                                                          \
        cJSON *p = cJSON_GetObjectItemCaseSensitive(json, #key);               \
        if (!cJSON_IsNumber(p) || NULL == p) {                                 \
            log_warn(c, "%s null or not number", #key);                        \
            return -1;                                                         \
        }                                                                      \
        log_debug(c, "%s=%f", #key, p->valuedouble);                           \
        dest = p->valuedouble;                                                 \
    }

#define cjson_def_get_fval_default(c, dest, json, key, val)                    \
    {                                                                          \
        cJSON *p = cJSON_GetObjectItemCaseSensitive(json, #key);               \
        if (!cJSON_IsNumber(p) || NULL == p) {                                 \
            log_debug(c, "%s null or not number", #key);                       \
            dest = val;                                                        \
        }                                                                      \
        else {                                                                 \
            dest = p->valuedouble;                                             \
        }                                                                      \
        log_debug(c, "%s=%f", #key, dest);                                     \
    }

#define cjson_def_conf_get_str_err(c, dest, json, key)                         \
    {                                                                          \
        cJSON *p = cJSON_GetObjectItemCaseSensitive(json, #key);               \
        if (!cJSON_IsString(p) || NULL == p->valuestring) {                    \
            CONF_LOG_WARN(c, "%s null or not string", #key);                   \
            return -1;                                                         \
        }                                                                      \
        dest = strdup(p->valuestring);                                         \
        if (dest == NULL) {                                                   \
            CONF_LOG_WARN(c, "%s strdup failed", #key);                      \
            return -1;                                                         \
        }                                                                      \
        CONF_LOG_DEBUG(c, "%s=%s ", #key, dest);                              \
    }

#define cjson_def_conf_get_str_default(c, dest, json, key, val)                \
    {                                                                          \
        cJSON *p = cJSON_GetObjectItemCaseSensitive(json, #key);               \
        if (!cJSON_IsString(p) || NULL == p->valuestring) {                    \
            if (NULL != val) {                                                 \
                CONF_LOG_DEBUG(c, "%s null or not string default %s", #key,    \
                               val);                                           \
                dest = strdup(val);                                            \
                if (dest == NULL) {                                            \
                    CONF_LOG_WARN(c, "%s strdup failed", #key);              \
                    return -1;                                                 \
                }                                                               \
            }                                                                  \
            else {                                                             \
                CONF_LOG_DEBUG(c, "%s null or not string default NULL", #key); \
                dest = NULL;                                                   \
            }                                                                  \
        }                                                                      \
        else {                                                                 \
            dest = strdup(p->valuestring);                                     \
            if (dest == NULL) {                                                \
                CONF_LOG_WARN(c, "%s strdup failed", #key);                  \
                return -1;                                                     \
            }                                                                  \
        }                                                                      \
        CONF_LOG_DEBUG(c, "%s=%s", #key, dest);                               \
    }

#define cjson_def_conf_get_ival_err(c, dest, json, key)                        \
    {                                                                          \
        cJSON *p = cJSON_GetObjectItemCaseSensitive(json, #key);               \
        if (!cJSON_IsNumber(p) || NULL == p) {                                 \
            CONF_LOG_WARN(c, "%s null or not number", #key);                   \
            return -1;                                                         \
        }                                                                      \
        CONF_LOG_DEBUG(c, "%s=%d", #key, p->valueint);                         \
        dest = p->valueint;                                                    \
    }

#define cjson_def_conf_get_ival_default(c, dest, json, key, val)               \
    {                                                                          \
        cJSON *p = cJSON_GetObjectItemCaseSensitive(json, #key);               \
        if (!cJSON_IsNumber(p) || NULL == p) {                                 \
            CONF_LOG_DEBUG(c, "%s null(%p) or not number", #key, p);           \
            dest = val;                                                        \
        }                                                                      \
        else {                                                                 \
            dest = p->valueint;                                                \
        }                                                                      \
        CONF_LOG_DEBUG(c, "%s=%d", #key, dest);                                \
    }

#define cjson_def_conf_get_fval_err(c, dest, json, key)                        \
    {                                                                          \
        cJSON *p = cJSON_GetObjectItemCaseSensitive(json, #key);               \
        if (!cJSON_IsNumber(p) || NULL == p) {                                 \
            CONF_LOG_WARN(c, "%s null or not number", #key);                   \
            return -1;                                                         \
        }                                                                      \
        CONF_LOG_DEBUG(c, "%s=%f", #key, p->valuedouble);                      \
        dest = p->valuedouble;                                                 \
    }

#define cjson_def_conf_get_fval_default(c, dest, json, key, val)               \
    {                                                                          \
        cJSON *p = cJSON_GetObjectItemCaseSensitive(json, #key);               \
        if (!cJSON_IsNumber(p) || NULL == p) {                                 \
            CONF_LOG_DEBUG(c, "%s null or not number", #key);                  \
            dest = val;                                                        \
        }                                                                      \
        else {                                                                 \
            dest = p->valuedouble;                                             \
        }                                                                      \
        CONF_LOG_DEBUG(c, "%s=%f", #key, dest);                                \
    }

#endif /* _CJSON_DEF_H_ */
