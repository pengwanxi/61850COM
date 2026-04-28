/**
 *   \file ied_data.h
 *   \brief iec61850 IED数据模型

 */
#ifndef _IED_DATA_H_
#define _IED_DATA_H_

#include "iec61850_server.h"

/**
 *  \brief 更新float 数据
 *  \param float
 *  \return ERR_OK-成功 其他-失败
 */
int ied_update_float(char *name, float fval);
int ied_update_ival(char *name, int ival);
int ied_update_sval(char *name, char *str);
int ied_update_bval(char *name, bool bval);
int ied_update_uival(char *name, unsigned int ival);
int ied_update_utctime(char *name, uint64_t ival);
int ied_update_dbpos(char *name, int dbpos);
int ied_update_quality(char *name, uint16_t valid);

int ied_update_all_quality(char *iedname, char *devname, uint16_t value);

/**
 *  \brief 获取sever
 *  \param void
 *  \return NULL-失败 其他-成功
 */
IedServer ied_server(void);

/**
 *  \brief 获取model
 *  \param void
 *  \return NULL-失败 其他-成功
 */
IedModel *ied_model(void);

int ied_data_init();
int ied_data_exit();




#endif /* _IED_DATA_H_ */
