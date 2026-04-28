/**
 *   \file dev_data.h
 *   \brief 装置数据
 */
#ifndef _DEV_DATA_H_
#define _DEV_DATA_H_

#include "dev_val.h"
#include "iec61850_conf.h"
#include "iec61850_macro.h"

/*  */
typedef enum _DEV_COMMSTATE {
    DEV_COMMSTATE_NORMAL = 0,
    DEV_COMMSTATE_NOTNORMAL,
} DEV_COMMSTATE;

/* 装置数据 */
typedef struct _DEV_DATA {
    char *devname;
    char *name;
    char *proto;
    int init;

    /* 装置独有 */
    char *port;
    char *addr;
    char guid[128];
    unsigned char commstate;

    IEC61850_CFG_TEMPLATE *ptemplate;

    int (*set_val)(struct _DEV_DATA *pdev, DEV_VAL *pval);
    int (*get_val)(struct _DEV_DATA *pdev, DEV_VAL *pval);
    int (*set_list_val)(struct _DEV_DATA *pdev, DEV_VAL_LIST *pval);
    int (*get_list_val)(struct _DEV_DATA *pdev, DEV_VAL_LIST *pval);
    int (*set_record)(struct _DEV_DATA *pdev, DEV_VAL *pval);
    int (*get_record)(struct _DEV_DATA *pdev, DEV_VAL *pval);

    void *priv_data;
} DEV_DATA;

int dev_data_register(DEV_DATA *pdev);
int dev_data_unregister(DEV_DATA *pdev);
int dev_data_init(DEV_DATA *pdev, char *name);
int dev_data_exit(DEV_DATA *pdev);

#endif /* _DEV_DATA_H_ */
