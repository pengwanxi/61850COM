#include <stdio.h>

#include "iec61850_config_json.h"
#include "file_operation.h"
#include "cJSON.h"
#include "err_def.h"
#include "cjson_def.h"
#include "iec61850_config_json.h"

static char *logname = "config_template_json";

/**
 *  \brief 解析数据
 *  \param cfg
 *  \param json
 *  \return err
 */
static int parse_iec61850_event(IEC61850_CONF *pcfg, int tno, int no,
                                cJSON *json)
{
    IEC61850_CFG_TEMPLATE *ptemplate = &pcfg->template_list.templates[tno];
    IEC61850_CFG_TEMPLATE_EVENT *pevent = &ptemplate->event_list.event[no];

    cjson_def_conf_get_str_err(logname, pevent->name, json, name);
    cjson_def_conf_get_str_err(logname, pevent->ref_name, json, ref_name);
    cjson_def_conf_get_str_err(logname, pevent->ref_time, json, ref_time);
    cjson_def_conf_get_str_default(logname, pevent->ref_q, json, ref_q, "");
    cjson_def_conf_get_str_err(logname, pevent->yx_idx, json, yx_idx);
    cjson_def_conf_get_str_default(logname, pevent->event_idx, json, event_idx, "");

    return ERR_OK;
}

/**
 *  \brief 解析数据列表
 *  \param cfg
 *  \param json
 *  \return err
 */
static int parse_iec61850_event_data_item(IEC61850_CONF *pcfg, int tno, int dno,
                                          int ino, cJSON *json)
{
    IEC61850_CFG_TEMPLATE_EVENT_DATA_ITEM *pitem =
        &pcfg->template_list.templates[tno].event_data_list.data[dno].items[ino];

    cjson_def_conf_get_str_err(logname, pitem->name, json, name);
    cjson_def_conf_get_str_err(logname, pitem->ref_name, json, ref_name);
    cjson_def_conf_get_str_default(logname, pitem->ref_time, json, ref_time, NULL);
    cjson_def_conf_get_str_default(logname, pitem->ref_q, json, ref_q, NULL);
    cjson_def_conf_get_str_err(logname, pitem->idx, json, idx);
    cjson_def_conf_get_str_default(logname, pitem->type, json, type, "float");

    return ERR_OK;
}

static int parse_iec61850_event_data(IEC61850_CONF *pcfg, int tno, int dno,
                                     cJSON *json)
{
    IEC61850_CFG_TEMPLATE_EVENT_DATA *pdata =
        &pcfg->template_list.templates[tno].event_data_list.data[dno];

    cjson_def_conf_get_str_err(logname, pdata->idx, json, idx);
    cjson_def_conf_get_str_err(logname, pdata->name, json, name);

    cJSON *items = cJSON_GetObjectItemCaseSensitive(json, "data");
    if (items) {
        cJSON *item;
        /* pdata->num = cJSON_GetArraySize(items); */
        cJSON_ArrayForEach(item, items)
        {
            if (pdata->num >= IEC61850_CFG_TEMPLATE_EVENT_DATA_MAX) {
                CONF_LOG_WARN(logname, "event_data items full %d", pdata->num);
                return -1;
            }
            int ret = parse_iec61850_event_data_item(pcfg, tno, dno,
                                                     pdata->num++, item);
            if (ret < 0) {
                CONF_LOG_WARN(logname, "parse_iec61850_event_data_item error");
                return -1;
            }
        }
    }

    return ERR_OK;
}

static int parse_iec61850_event_data_list(IEC61850_CONF *pcfg, int no,
                                          cJSON *json)
{
    IEC61850_CFG_TEMPLATE_EVENT_DATA_LIST *plist =
        &pcfg->template_list.templates[no].event_data_list;

    cJSON *ele;
    cJSON_ArrayForEach(ele, json)
    {
        if (plist->num >= IEC61850_CFG_TEMPLATE_EVENT_DATA_MAX) {
            CONF_LOG_WARN(logname, "event_data_list full %d", plist->num);
            return -1;
        }

        int ret = parse_iec61850_event_data(pcfg, no, plist->num++, ele);
        if (ret < 0) {
            CONF_LOG_WARN(logname, "parse_iec61850_event_data error");
            return -1;
        }
    }

    return ERR_OK;
}

static int parse_iec61850_event_list(IEC61850_CONF *pcfg, int no, cJSON *json)
{
    IEC61850_CFG_TEMPLATE *ptemplate = &pcfg->template_list.templates[no];
    IEC61850_CFG_TEMPLATE_EVENT_LIST *peventlist = &ptemplate->event_list;
    cJSON *ele;
    cJSON_ArrayForEach(ele, json)
    {
        if (peventlist->num >= IEC61850_CFG_TEMPLATE_EVENT_MAX) {
            CONF_LOG_WARN(logname, "event_list full %d", peventlist->num);
            return -1;
        }

        int ret = parse_iec61850_event(pcfg, no, peventlist->num++, ele);
        if (ret < 0) {
            CONF_LOG_WARN(logname, "parse_iec61850_dev error");
            return -1;
        }
    }

    return ERR_OK;
}

/**
 *  \brief 解析数据
 *  \param cfg
 *  \param json
 *  \return err
 */
static int parse_iec61850_dataset(IEC61850_CONF *pcfg, int tno, int no,
                                  cJSON *json)
{
    IEC61850_CFG_TEMPLATE *ptemplate = &pcfg->template_list.templates[tno];
    IEC61850_CFG_TEMPLATE_DATASET *pdataset =
        &ptemplate->dataset_list.dataset[no];

    cjson_def_conf_get_str_err(logname, pdataset->name, json, name);
    cjson_def_conf_get_str_default(logname, pdataset->name_desc, json,
                                   name_desc, pdataset->name);
    cjson_def_conf_get_str_default(logname, pdataset->ref_name, json, ref_name, NULL);
    cjson_def_conf_get_str_default(logname, pdataset->ref_time, json, ref_time, NULL);
    cjson_def_conf_get_str_default(logname, pdataset->ref_q, json, ref_q, NULL);

    if (0 == strcmp(ptemplate->proto, TEMPLATE_PROTO_EMU2000_SHM)) {
        cjson_def_conf_get_str_err(logname, pdataset->data_type, json,
                                   data_type);
        cjson_def_conf_get_str_err(logname, pdataset->data_idx, json, data_idx);
    }
    else if (0 == strcmp(ptemplate->proto, TEMPLATE_PROTO_LOCAL_IEC61850)) {
        cjson_def_conf_get_str_err(logname, pdataset->data_type, json,
                                   data_type);
        cjson_def_conf_get_str_err(logname, pdataset->data_idx, json, data_idx);
    }
    else {
        return ERR_TYPE;
    }

    cjson_def_conf_get_str_default(logname, pdataset->data_desc, json,
                                   data_desc, "");
    cjson_def_conf_get_str_default(logname, pdataset->db_name, json, db_name,
                                   pdataset->name);

    cjson_def_conf_get_str_default(logname, pdataset->ratio, json, ratio, "1");

    return ERR_OK;
}

/**
 *  \brief 解析数据列表
 *  \param cfg
 *  \param json
 *  \return err
 */
static int parse_iec61850_dataset_list(IEC61850_CONF *pcfg, int no, cJSON *json)
{
    IEC61850_CFG_TEMPLATE *ptemplate = &pcfg->template_list.templates[no];
    IEC61850_CFG_TEMPLATE_DATASET_LIST *pdatasetlist = &ptemplate->dataset_list;
    cJSON *ele;
    cJSON_ArrayForEach(ele, json)
    {
        if (pdatasetlist->num >= IEC61850_CFG_TEMPLATE_DATASET_MAX) {
            CONF_LOG_WARN(logname, "dataset_list full %d", pdatasetlist->num);
            return -1;
        }

        int ret = parse_iec61850_dataset(pcfg, no, pdatasetlist->num++, ele);
        if (ret < 0) {
            CONF_LOG_WARN(logname, "parse_iec61850_dev error");
            return -1;
        }
    }

    return ERR_OK;
}

static int parse_iec61850_model(IEC61850_CONF *pcfg, int tno, cJSON *json)
{
    IEC61850_CFG_TEMPLATE_MODEL *pmodel =
        &pcfg->template_list.templates[tno].model;

    cjson_def_conf_get_str_err(logname, pmodel->model, json, model);
    cjson_def_conf_get_str_err(logname, pmodel->desc, json, desc);
    cjson_def_conf_get_str_err(logname, pmodel->proType, json, proType);
    cjson_def_conf_get_str_err(logname, pmodel->manuID, json, manuID);
    cjson_def_conf_get_str_err(logname, pmodel->isreport, json, isreport);

    return ERR_OK;
}

static int parse_iec61850_lln0(IEC61850_CONF *pcfg, int tno, cJSON *json)
{
    IEC61850_CFG_LLN0_NAMPLT *pnamplt =
        &pcfg->template_list.templates[tno].lln0.NamPlt;

    cJSON *namplt = cJSON_GetObjectItemCaseSensitive(json, "NamPlt");
    if (namplt) {
        cjson_def_conf_get_str_default(logname, pnamplt->vendor, namplt, vendor,
                                       "");
        cjson_def_conf_get_str_default(logname, pnamplt->swRev, namplt, swRev,
                                       "");
        cjson_def_conf_get_str_default(logname, pnamplt->d, namplt, d, "");
        cjson_def_conf_get_str_default(logname, pnamplt->configRev, namplt,
                                       configRev, "");
        cjson_def_conf_get_str_default(logname, pnamplt->ldNs, namplt, ldNs,
                                       "");
    }

    return ERR_OK;
}

/**
 *  \brief 解析模板列表
 *  \param cfg
 *  \param json
 *  \return err
 */
static int parse_iec61850_template(IEC61850_CONF *pcfg, int no, cJSON *json)
{
    IEC61850_CFG_TEMPLATE *ptemplate = &pcfg->template_list.templates[no];

    cjson_def_conf_get_str_err(logname, ptemplate->name, json, name);
    cjson_def_conf_get_str_err(logname, ptemplate->version, json, version);
    cjson_def_conf_get_str_err(logname, ptemplate->proto, json, proto);
    cjson_def_conf_get_str_err(logname, ptemplate->desc, json, desc);
    cjson_def_conf_get_str_default(logname, ptemplate->ldname, json, ldname,
                                   ptemplate->name);
    cjson_def_conf_get_str_default(logname, ptemplate->cfgname, json, cfgname,
                                   ptemplate->name);

    if (TEMPLATE_PROTO_MATCH_SCU_JSON(ptemplate->proto)) {
        cJSON *model = cJSON_GetObjectItemCaseSensitive(json, "model");
        if (NULL != model) {
            if (parse_iec61850_model(pcfg, no, model) < 0) {
                CONF_LOG_WARN(logname, "parse_iec61850_model error");
                return -1;
            }
        }
    }

    cJSON *lln0 = cJSON_GetObjectItemCaseSensitive(json, "LLN0");
    if (NULL != lln0) {
        if (parse_iec61850_lln0(pcfg, no, lln0) < 0) {
            CONF_LOG_WARN(logname, "parse_iec61850_lln0 error");
            return -1;
        }
    }

    cJSON *dataset = cJSON_GetObjectItemCaseSensitive(json, "dataset");
    if (NULL != dataset) {
        if (parse_iec61850_dataset_list(pcfg, no, dataset) < 0) {
            CONF_LOG_WARN(logname, "parse_iec61850_dataset error");
            return -1;
        }
    }

    cJSON *event = cJSON_GetObjectItemCaseSensitive(json, "event");
    if (NULL != event) {
        if (parse_iec61850_event_list(pcfg, no, event) < 0) {
            CONF_LOG_WARN(logname, "parse_iec61850_dataset error");
            return -1;
        }
    }

    cJSON *event_data = cJSON_GetObjectItemCaseSensitive(json, "event_data");
    if (NULL != event_data) {
        if (parse_iec61850_event_data_list(pcfg, no, event_data) < 0) {
            CONF_LOG_WARN(logname, "parse_iec61850_event_data_list error");
            return -1;
        }
    }

    return ERR_OK;
}

/**
 *  \brief 解析模板列表
 *  \param cfg
 *  \param json
 *  \return err
 */
static int parse_iec61850_template_list(IEC61850_CONF *pcfg, cJSON *json)
{
    char *name = NULL;
    cjson_def_conf_get_str_err(logname, name, json, name);
    if (name == NULL) {
        CONF_LOG_WARN(logname, "template name null");
        return ERR_PNULL;
    }

    for (int i = 0; i < pcfg->template_list.num; i++) {
        if (pcfg->template_list.templates[i].name != NULL &&
            0 == strcmp(pcfg->template_list.templates[i].name, name)) {
            free(name);
            return ERR_OK;
        }
    }

    if (pcfg->template_list.num >= IEC61850_CFG_TEMPLATE_MAX) {
        CONF_LOG_WARN(logname, "template_list full %d",
                      pcfg->template_list.num);
        free(name);
        return -1;
    }

    free(name);
    return parse_iec61850_template(pcfg, pcfg->template_list.num++, json);
}

int iec61850_config_template_json(IEC61850_CONF *pcfg, char *filename)
{
    return iec61850_config_read_json(pcfg, filename,
                                     parse_iec61850_template_list);
}
