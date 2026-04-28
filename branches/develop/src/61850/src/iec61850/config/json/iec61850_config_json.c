#include <stdio.h>

#include "iec61850_config_json.h"
#include "file_operation.h"
#include "err_def.h"
#include "cjson_def.h"
#include "iec61850_template_json.h"

static char *logname = "config_json";

/**
 *  \brief 读json
 *  \param data
 *  \param parse
 *  \return err
 */
int iec61850_config_read_json(IEC61850_CONF *pcfg, char *filename,
                     int (*parse)(IEC61850_CONF *pcfg, cJSON *json))
{
    if (NULL == pcfg || NULL == filename || NULL == parse) {
        return ERR_PTR;
    }

    char path[128];
    char *text;
    int len;

    snprintf(path, 128, "%s", filename);

    text = file_operation_read_malloc(path, &len);
    if (NULL == text) {
        CONF_LOG_WARN(logname, "file_operation_read_malloc %s error", path);
        return -1;
    }

    cJSON *json = NULL;
    json = cJSON_Parse(text);
    if (NULL == json) {
        file_operation_read_free(&text);
        CONF_LOG_WARN(logname, "cJSON_Parse %s error", path);
        return -1;
    }

    if (parse(pcfg, json) < 0) {
        cJSON_Delete(json);
        file_operation_read_free(&text);
        CONF_LOG_WARN(logname, "parse %s error", path);
        return -1;
    }

    cJSON_Delete(json);
    file_operation_read_free(&text);
    /* CONF_LOG_INFO(logname, "%s ok", path); */

    return ERR_OK;
}


/**
 *  \brief 解析装置
 *  \param cfg
 *  \param json
 *  \return err
 */
static int parse_iec61850_dev(IEC61850_CONF *pcfg, int no, cJSON *json)
{
    IEC61850_CFG_DEV *pdev = &pcfg->devlist.devs[no];

    cjson_def_conf_get_str_err(logname, pdev->port, json, port);
    cjson_def_conf_get_str_err(logname, pdev->addr, json, addr);

    /* 61850 模型 */
    cjson_def_conf_get_str_err(logname, pdev->devname, json, devname);
    /* cjson_def_conf_get_str_err(logname, pdev->devmodel, json, devmodel); */
    /* ref */
    cjson_def_conf_get_str_err(logname, pdev->template_name, json, template);

    /* 类型 */
    cjson_def_conf_get_str_err(logname, pdev->type, json, type);

    if (pdev->template_name) {
        char filename[128];
        snprintf(filename, sizeof(filename), "%s/template/%s.json", CONFIG_PATH,
                 pdev->template_name);
        return iec61850_config_template_json(pcfg, filename);
    }

    return ERR_OK;
}

/**
 *  \brief 解析装置列表
 *  \param cfg
 *  \param json
 *  \return err
 */
static int parse_iec61850_devlist(IEC61850_CONF *pcfg, cJSON *json)
{
    IEC61850_CFG_DEVLIST *pdevlist = &pcfg->devlist;

    cJSON *ele;
    cJSON_ArrayForEach(ele, json)
    {
        if (pdevlist->num >= IEC61850_CFG_DEV_MAX) {
            CONF_LOG_WARN(logname, "devlist full %d", pdevlist->num);
            return -1;
        }

        int ret = parse_iec61850_dev(pcfg, pdevlist->num++, ele);
        if (ret < 0) {
            CONF_LOG_WARN(logname, "parse_iec61850_dev error");
            return -1;
        }
    }

    return ERR_OK;
}

/**
 *  \brief 解析内容
 *  \param cfg
 *  \param json
 *  \return err
 */
static int parse_iec61850(IEC61850_CONF *pcfg, cJSON *json)
{
    CONF_LOG_INFO(logname, "parse_iec61850 start");

    cjson_def_conf_get_str_err(logname, pcfg->name, json, name);

    cjson_def_conf_get_str_err(logname, pcfg->version, json, version);

    cJSON *devlist = cJSON_GetObjectItemCaseSensitive(json, "devlist");
    if (NULL != devlist) {
        if (parse_iec61850_devlist(pcfg, devlist) < 0) {
            CONF_LOG_WARN(logname, "parse_iec61850_devlist error");
            return -1;
        }
    }

    return ERR_OK;
}

int iec61850_config_json_parse(IEC61850_CONF *pcfg, char *filename)
{
    return iec61850_config_read_json(pcfg, filename, parse_iec61850);
}
