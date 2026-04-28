/**
 *   \file dev_list.h
 *   \brief 装置列表
 */
#ifndef _DEV_LIST_H_
#define _DEV_LIST_H_

#include "dev_data.h"

DEV_DATA* dev_list_ied_data(void);
DEV_DATA* dev_list_data(char *name);
DEV_DATA *dev_list_data_byguid(char *guid);
void* dev_list_private_data(char *name);

int dev_list_init(void);
int dev_list_exit(void);

#endif /* _DEV_LIST_H_ */
