/**
 *   \file model_ied.h
 *   \brief IECied 模型模板
 */
#ifndef _MODEL_IED_H_
#define _MODEL_IED_H_

#include "iec61850_server.h"

/* IECied 模型私有数据 */
typedef struct _MODEL_IED_PRIVATE_DATA {
    IedModel *model;

} MODEL_IED_PRIVATE_DATA;

int model_ied_init();
int model_ied_exit();

#endif /* _MODEL_IED_H_ */
