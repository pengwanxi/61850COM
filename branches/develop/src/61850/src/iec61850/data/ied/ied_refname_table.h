/**
 *   \file ied_refname_table.h
 *   \brief 模型基本的索引名称
 */
#ifndef _IED_REFNAME_TABLE_H_
#define _IED_REFNAME_TABLE_H_

#include "iec61850_server.h"
#include "iec61850_macro.h"

/*  */
typedef struct _IED_REFNAME {
    char *refname;
} IED_REFNAME;

/*  */
typedef struct _IED_REFNAME_TABLE {
    char *name;
    int num;
    IED_REFNAME refnames[IED_REFNAME_MAX];
} IED_REFNAME_TABLE;

/* 索引名称表 */
typedef struct _IED_REFNAME_TABLE_LIST {
    int num;
    IED_REFNAME_TABLE tables[IED_REFNAME_TABLE_MAX];

} IED_REFNAME_TABLE_LIST;

/**
 *  \brief 初始化
 *  \param IED_REFNAME_TABLE_LIST
 *  \param IED_MODEL
 *  \return ERR_OK-成功 其他-失败
 */
int ied_refname_table_init(IED_REFNAME_TABLE_LIST *ptable_list,
                            IedModel *ied_model);
int ied_refname_table_exit(IED_REFNAME_TABLE_LIST *ptable_list);

void ied_refname_table_print(IED_REFNAME_TABLE_LIST *ptable_list);
void ied_refname_table_print_json(IED_REFNAME_TABLE_LIST *ptable_list);
void ied_refname_table_write_json(IED_REFNAME_TABLE_LIST *ptable_list);

#endif /* _IED_REFNAME_TABLE_H_ */
