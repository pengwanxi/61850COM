/**
 *   \file iec61850_config.h
 *   \brief 61850配置
 */
#ifndef _IEC61850_CONF_H_
#define _IEC61850_CONF_H_

#include "iec61850_model.h"
#include "iec61850_macro.h"

/*  */
typedef struct _MODEL_IDX {
    char *refname;
    DataAttribute *pattr;
} MODEL_IDX;

#define IEC61850_CFG_TEMPLATE_EVENT_DATA_MAX (32)
/* 模版对应点  */
typedef struct _IEC61850_CFG_TEMPLATE_EVENT_DATA_ITEM {
    char *name;
    char *ref_name;
    char *ref_time;
    char *ref_q;
    char *type;
    char *idx;
} IEC61850_CFG_TEMPLATE_EVENT_DATA_ITEM;

typedef struct _IEC61850_CFG_TEMPLATE_EVENT_DATA {
    char *idx;
    char *name;
    int num;
    IEC61850_CFG_TEMPLATE_EVENT_DATA_ITEM
        items[IEC61850_CFG_TEMPLATE_EVENT_DATA_MAX];
} IEC61850_CFG_TEMPLATE_EVENT_DATA;

typedef struct _IEC61850_CFG_TEMPLATE_EVENT_DATA_LIST {
    int num;
    IEC61850_CFG_TEMPLATE_EVENT_DATA data[IEC61850_CFG_TEMPLATE_EVENT_MAX];
} IEC61850_CFG_TEMPLATE_EVENT_DATA_LIST;

typedef struct _IEC61850_CFG_TEMPLATE_EVENT {
    /* 点号惟一名称 */
    char *name;
    /* 对应61850 */
    char *ref_name;
    char *ref_time;

    char *ref_q;

    /* 对应点号配置 */
    char *yx_idx;
    /* 对应event_data的索引 */
    char *event_idx;
} IEC61850_CFG_TEMPLATE_EVENT;

/* 模版对应点  */
typedef struct _IEC61850_CFG_TEMPLATE_EVNET_LIST {
    int num;
    IEC61850_CFG_TEMPLATE_EVENT event[IEC61850_CFG_TEMPLATE_EVENT_MAX];
} IEC61850_CFG_TEMPLATE_EVENT_LIST;

/* 模版对应点  */
typedef struct _IEC61850_CFG_TEMPLATE_DATASET {
    /* 点号惟一名称 */
    char *name;
    char *name_desc;

    /* 对应61850 */
    char *ref_name;
    char *ref_time;
    char *ref_q;

    /* 对应数据中心，如此处NULL,则和name 名称一 */
    char *db_name;

    /* 对应点号配置 */
    char *data_type;
    char *data_idx;
    char *data_desc;

    char *ratio;

} IEC61850_CFG_TEMPLATE_DATASET;

/* 模版对应点  */
typedef struct _IEC61850_CFG_TEMPLATE_DATASET_LIST {
    int num;
    IEC61850_CFG_TEMPLATE_DATASET dataset[IEC61850_CFG_TEMPLATE_DATASET_MAX];
} IEC61850_CFG_TEMPLATE_DATASET_LIST;

/*  */
typedef struct _IEC61850_CFG_TEMPLATE_MODEL {
    char *model;
    char *desc;
    char *proType;
    char *manuID;
    char *isreport;
} IEC61850_CFG_TEMPLATE_MODEL;

/*  */
typedef struct _IEC61850_CFG_LLN0_NAMPLT {
    char *vendor;
    char *swRev;
    char *d;
    char *configRev;
    char *ldNs;
} IEC61850_CFG_LLN0_NAMPLT;

typedef struct _IEC61850_CFG_LLN0 {
    IEC61850_CFG_LLN0_NAMPLT NamPlt;
} IEC61850_CFG_LLN0;

typedef struct _IEC61850_CFG_TEMPLATE {
    char *name;
    char *proto;
    char *version;
    char *desc;
    char *cfgname; /* 配置文件名称 */
    char *ldname; /* 逻辑名称 */

    IEC61850_CFG_TEMPLATE_MODEL model;
    IEC61850_CFG_LLN0 lln0;
    IEC61850_CFG_TEMPLATE_DATASET_LIST dataset_list;
    IEC61850_CFG_TEMPLATE_EVENT_LIST event_list;
    IEC61850_CFG_TEMPLATE_EVENT_DATA_LIST event_data_list;

} IEC61850_CFG_TEMPLATE;

/*  */
typedef struct _IEC61850_CFG_TEMPLATE_LIST {
    int num;
    IEC61850_CFG_TEMPLATE templates[IEC61850_CFG_TEMPLATE_MAX];
} IEC61850_CFG_TEMPLATE_LIST;

/*  */
typedef struct _IEC61850_CFG_DEV {
    /* port addr 用在所有协议，其它的用在数据中心 */
    char *port;
    char *addr;

    /* 61850模块的名称及模块类型 */
    char *devname;

    /* 61850的ref_name与其它协议模板的点对应 */
    char *template_name;

    char *type;
} IEC61850_CFG_DEV;

/*  */
typedef struct _IEC61850_CFG_DEVLIST {
    int num;
    IEC61850_CFG_DEV devs[IEC61850_CFG_DEV_MAX];
} IEC61850_CFG_DEVLIST;

/*  */
typedef struct _IEC61850_CONF {
    char *name;
    char *version;

    IEC61850_CFG_DEVLIST devlist;
    IEC61850_CFG_TEMPLATE_LIST template_list;

    void *pmodel;
    void *pmodel_idx_list;
    void *pied_server;
} IEC61850_CONF;

/**
 *  \brief 查找装置
 *  \param IEC61850_CONF
 *  \param name
 *  \return IEC61850_CFG_DEV*
 */
IEC61850_CFG_DEV *iec61850_config_dev(IEC61850_CONF *pcfg, char *name);

/**
 *  \brief 查找模板
 *  \param IEC61850_CONF
 *  \param name
 *  \return IEC61850_CFG_TEMPLATE *
 */
IEC61850_CFG_TEMPLATE *iec61850_config_template(IEC61850_CONF *pcfg,
                                                char *name);

/**
 *  \brief 初始化配置
 *  \param name
 *  \return errCode
 */
int iec61850_config_init(char *name, IEC61850_CONF **pcfg);

/**
 *  \brief 初始化退出
 *  \return errCode
 */
int iec61850_config_exit(IEC61850_CONF *pcfg);

#endif /* _IEC61850_CONF_H_ */
