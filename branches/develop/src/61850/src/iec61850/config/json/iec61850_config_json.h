/**
 *   \file iec61850_config_json.h
 *   \brief json 配置文件
 */
#ifndef _IEC61850_CONFIG_JSON_H_
#define _IEC61850_CONFIG_JSON_H_

#include "iec61850_conf.h"
#include "cJSON.h"
/**
 *  \brief 读iec61850.json
 *  \param filename 文件名
 *  \return err
 */
int iec61850_config_json_parse(IEC61850_CONF *pcfg, char *filename);

int iec61850_config_read_json(IEC61850_CONF *pcfg, char *filename,
                              int (*parse)(IEC61850_CONF *pcfg, cJSON *json));

#endif /* _IEC61850_CONFIG_JSON_H_ */
