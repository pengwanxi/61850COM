/**
 *   \file dev_val_set.h
 *   \brief 设置装置数据
 */
#ifndef _DEV_VAL_SET_H_
#define _DEV_VAL_SET_H_

#include "dev_val.h"
#include "dev_data.h"

/**
 *  \brief 设置装置数据
 *  \param *pdev
 *  \return err
 */
int set_dev_data(DEV_DATA *pdev, DEV_VAL *pval);

#endif /* _DEV_VAL_SET_H_ */
