/**
 *   \file iec61850_template_json.h
 *   \brief 用json 的方式读模型
 */
#ifndef _IEC61850_TEMPLATE_JSON_H_
#define _IEC61850_TEMPLATE_JSON_H_

#include "iec61850_conf.h"

/**
 *  \brief 读template下的
 *  \param filename 文件名
 *  \return err
 */
int iec61850_config_template_json(IEC61850_CONF *pcfg, char *filename);

#endif /* _IEC61850_TEMPLATE_JSON_H_ */
