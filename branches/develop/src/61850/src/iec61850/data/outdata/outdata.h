/**
 *   \file outdata.h
 *   \brief 外部数据
 *
 *  主要是数据中心或是共享内存的数据
 *
 */
#ifndef _OUTDATA_H_
#define _OUTDATA_H_

#include <stdbool.h>
#include <stdint.h>
#include "dev_data.h"
#include "outval.h"

/*  */
typedef enum _OUTDATA_TYPE {
    ODT_NONE = 0, /*  */
    ODT_EMU2000_SHM,
    ODT_SCU_JSON,
} OUTDATA_TYPE;

/*  */
typedef struct _OUTDATA {
    int type;

    int (*get_val)(struct _OUTDATA *pdata, OUTVAL *pval);
    int (*get_val_list)(struct _OUTDATA *pdata, OUTVAL_LIST *pval);
    int (*set_val)(struct _OUTDATA *pdata, OUTVAL *pval);
    int (*set_val_list)(struct _OUTDATA *pdata, OUTVAL_LIST *pval);

    void *priv_data;
} OUTDATA;


OUTDATA *outdata_get(int type);

/**
 *  \brief 初始化
 *  \param void
 *  \return ERR_OK 成功
 */
int outdata_init();
int outdata_exit();

#endif /* _OUTDATA_H_ */
