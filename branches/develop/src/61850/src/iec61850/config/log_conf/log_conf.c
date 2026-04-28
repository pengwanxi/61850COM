#include "log_conf.h"
#include "../file_operation/file_operation.h"
#include "cJSON.h"

static int gs_log_conf_enable = 0;
static struct stat log_json_stat;
static char *log_json = LOG_CLOG_PATH;
static int log_conf_enbale = 1;

void log_conf_set_enable(int enable)
{
    gs_log_conf_enable = enable;
}

int log_conf_enable(void)
{
    return gs_log_conf_enable;
}

typedef struct _LOG_CONF_DATA {
    char conf_name[128];

    int default_level;
    int default_timeout_s;

    int glog_num;
    LOG_LIST_UNIT_CLASS *glog_data;

    int protocol_num;
    LOG_LIST_UNIT_CLASS *protocol_data;

    int port_num;
    LOG_LIST_UNIT_CLASS *port_data;

    int dev_num;
    LOG_LIST_UNIT_CLASS *dev_data;

    int deval_num;
    LOG_LIST_UNIT_CLASS *deval_data;

    int database_num;
    LOG_LIST_UNIT_CLASS *database_data;
} LOG_CONF_DATA;

int log_conf_read(char *log_json);
int log_conf_read_free(void);
LOG_CONF_DATA *log_conf_get_ptr(void);
void log_conf_display(int log_level);

LOG_CONF_DATA gs_log_conf;

static int log_conf_read_category(cJSON *items, LOG_LIST_UNIT_CLASS **pdata,
                                  int *num)
{
    if (pdata == NULL || num == NULL) {
        return -1;
    }

    *pdata = NULL;
    *num = 0;

    if (items == NULL || !cJSON_IsArray(items)) {
        return -1;
    }

    int array_size = cJSON_GetArraySize(items);
    if (array_size <= 0) {
        return 0;
    }

    *pdata = (LOG_LIST_UNIT_CLASS *)malloc(sizeof(LOG_LIST_UNIT_CLASS) * array_size);
    if (*pdata == NULL) {
        return -1;
    }
    memset(*pdata, 0, sizeof(LOG_LIST_UNIT_CLASS) * array_size);
    *num = array_size;

    for (int i = 0; i < *num; i++) {
        cJSON *item = cJSON_GetArrayItem(items, i);
        LOG_LIST_UNIT_CLASS *pitem = &(*pdata)[i];

        cJSON *name = cJSON_GetObjectItemCaseSensitive(item, "name");
        if (!cJSON_IsString(name) || !name->valuestring) {
            goto err;
        }
        if (strlen(name->valuestring) >= sizeof(pitem->name)) {
            goto err;
        }
        snprintf(pitem->name, sizeof(pitem->name), "%s", name->valuestring);

        cJSON *level = cJSON_GetObjectItemCaseSensitive(item, "level");
        if (!cJSON_IsNumber(level)) {
            goto err;
        }
        pitem->level = level->valueint;
    }

    return 0;

err:
    if (*pdata) {
        free(*pdata);
        *pdata = NULL;
    }
    *num = 0;
    return -1;
}

int log_conf_read(char *log_json)
{
    char *text;
    int len;
    memset(&gs_log_conf, 0, sizeof(LOG_CONF_DATA));

    snprintf(gs_log_conf.conf_name, 128, "%s", log_json);

    text = file_operation_read_malloc(gs_log_conf.conf_name, &len);
    if (!text) {
        return -1;
    }

    cJSON *json = cJSON_Parse(text);
    file_operation_read_free(&text);
    if (!json) {
        log_warn(GLOG, "%s read error", gs_log_conf.conf_name);
        return -1;
    }

    cJSON *default_level = cJSON_GetObjectItemCaseSensitive(json, "default_"
                                                                  "level");
    if (cJSON_IsNumber(default_level)) {
        gs_log_conf.default_level = default_level->valueint;
    }

    cJSON *default_timeout = cJSON_GetObjectItemCaseSensitive(json, "default_"
                                                                    "timeout_"
                                                                    "s");
    if (cJSON_IsNumber(default_timeout)) {
        gs_log_conf.default_timeout_s = default_timeout->valueint;
    }

    cJSON *protocol = cJSON_GetObjectItemCaseSensitive(json, "protocol");
    if (log_conf_read_category(protocol, &gs_log_conf.protocol_data,
                               &gs_log_conf.protocol_num) < 0) {
        cJSON_Delete(json);
        log_warn(GLOG, "%s read protocol error", gs_log_conf.conf_name);
        log_conf_read_free();
        return -2;
    }

    cJSON *port = cJSON_GetObjectItemCaseSensitive(json, "port");
    if (log_conf_read_category(port, &gs_log_conf.port_data,
                               &gs_log_conf.port_num) < 0) {
        cJSON_Delete(json);
        log_warn(GLOG, "%s read port error", gs_log_conf.conf_name);
        log_conf_read_free();
        return -3;
    }

    cJSON *dev = cJSON_GetObjectItemCaseSensitive(json, "dev");
    if (log_conf_read_category(dev, &gs_log_conf.dev_data,
                               &gs_log_conf.dev_num) < 0) {
        cJSON_Delete(json);
        log_warn(GLOG, "%s read dev error", gs_log_conf.conf_name);
        log_conf_read_free();
        return -4;
    }

    cJSON *glog = cJSON_GetObjectItemCaseSensitive(json, "glog");
    if (log_conf_read_category(glog, &gs_log_conf.glog_data,
                               &gs_log_conf.glog_num) < 0) {
        cJSON_Delete(json);
        log_warn(GLOG, "%s read glog error", gs_log_conf.conf_name);
        log_conf_read_free();
        return -5;
    }

    cJSON *database = cJSON_GetObjectItemCaseSensitive(json, "database");
    if (log_conf_read_category(database, &gs_log_conf.database_data,
                               &gs_log_conf.database_num) < 0) {
        cJSON_Delete(json);
        log_warn(GLOG, "%s read database error", gs_log_conf.conf_name);
        log_conf_read_free();
        return -6;
    }

    cJSON_Delete(json);
    return 0;
}

int log_conf_read_free(void)
{
    if (gs_log_conf.glog_data) {
        free(gs_log_conf.glog_data);
        gs_log_conf.glog_num = 0;
        gs_log_conf.glog_data = NULL;
    }
    if (gs_log_conf.protocol_data) {
        free(gs_log_conf.protocol_data);
        gs_log_conf.protocol_num = 0;
        gs_log_conf.protocol_data = NULL;
    }
    if (gs_log_conf.port_data) {
        gs_log_conf.port_num = 0;
        free(gs_log_conf.port_data);
        gs_log_conf.port_data = NULL;
    }
    if (gs_log_conf.dev_data) {
        gs_log_conf.dev_num = 0;
        free(gs_log_conf.dev_data);
        gs_log_conf.dev_data = NULL;
    }
    if (gs_log_conf.deval_data) {
        gs_log_conf.deval_num = 0;
        free(gs_log_conf.deval_data);
        gs_log_conf.deval_data = NULL;
    }
    if (gs_log_conf.database_data) {
        gs_log_conf.database_num = 0;
        free(gs_log_conf.database_data);
        gs_log_conf.database_data = NULL;
    }
    return 0;
}

LOG_CONF_DATA *log_conf_get_ptr(void)
{
    return &gs_log_conf;
}

void log_conf_display(int log_level)
{
    LOG_CONF_DATA *pdata = log_conf_get_ptr();

    for (int i = 0; i < pdata->glog_num; i++) {
        LOG_LIST_UNIT_CLASS *p = &pdata->glog_data[i];
        log_info(log_level, "glog %s: level=%d", p->name, p->level);
    }

    for (int i = 0; i < pdata->protocol_num; i++) {
        LOG_LIST_UNIT_CLASS *p = &pdata->protocol_data[i];
        log_info(log_level, "protocol %s: level=%d", p->name, p->level);
    }

    for (int i = 0; i < pdata->port_num; i++) {
        LOG_LIST_UNIT_CLASS *p = &pdata->port_data[i];
        log_info(log_level, "port %s: level=%d", p->name, p->level);
    }

    for (int i = 0; i < pdata->dev_num; i++) {
        LOG_LIST_UNIT_CLASS *p = &pdata->dev_data[i];
        log_info(log_level, "dev %s: level=%d", p->name, p->level);
    }

    for (int i = 0; i < pdata->deval_num; i++) {
        LOG_LIST_UNIT_CLASS *p = &pdata->deval_data[i];
        log_info(log_level, "deval %s: level=%d", p->name, p->level);
    }

    for (int i = 0; i < pdata->database_num; i++) {
        LOG_LIST_UNIT_CLASS *p = &pdata->database_data[i];
        log_info(log_level, "database %s: level=%d", p->name, p->level);
    }
}

int log_conf_init()
{
    if (file_operation_exist(log_json)) {
        if (file_operation_is_change(log_json, &log_json_stat)) {
            if (log_conf_read(log_json) == 0) {
                log_conf_display(GLOG);
                LOG_CONF_DATA *conf = log_conf_get_ptr();
                int ret;

                // 设置协议日志分类
                if (conf->protocol_num > 0) {
                    ret = log_set_cname(PROLOG, conf->protocol_num,
                                        conf->protocol_data);
                    log_info(GLOG, "log_set_cname protocol err=%d", ret);
                }

                // 设置端口日志分类
                if (conf->port_num > 0) {
                    ret =
                        log_set_cname(COMMLOG, conf->port_num, conf->port_data);
                    log_info(GLOG, "log_set_cname port err=%d", ret);
                }

                // 设置设备日志分类
                if (conf->dev_num > 0) {
                    ret = log_set_cname(DEVLOG, conf->dev_num, conf->dev_data);
                    log_info(GLOG, "log_set_cname dev err=%d", ret);
                    ret = log_set_cname(DVALOG, conf->dev_num, conf->dev_data);
                    log_info(GLOG, "log_set_cname dev_val err=%d", ret);
                }

                // 设置全局日志分类
                if (conf->glog_num > 0) {
                    ret = log_set_cname(GLOG, conf->glog_num, conf->glog_data);
                    log_info(GLOG, "log_set_cname conf err=%d", ret);
                }

                log_conf_read_free();
                log_conf_enbale = 1;
                log_info(GLOG, "log_conf_enable=%d", log_conf_enbale);
            }
        }
    }
    else {
        log_conf_enbale = 0;
        log_info(GLOG, "log_conf_enable=%d", log_conf_enbale);
    }
    log_conf_set_enable(log_conf_enbale);

    return 0;
}
