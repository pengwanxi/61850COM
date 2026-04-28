/**
 *   \file svc_list.h
 *   \brief 服务列表
 */
#ifndef _SVC_LIST_H_
#define _SVC_LIST_H_

#include <pthread.h>

/*  */
typedef struct _SVC_DATA
{
    char *name;                 /* 服务名称 */

    int (*init)(void);        /* 初始化 */
    int (*exit)(void);        /* 退出 */
    int (*run)(void);         /* 运行 */
    int (*stop)(void);         /* 停止 */

}SVC_DATA;



int svc_list_init(void);
int svc_list_exit(void);
int svc_list_run(void);
int svc_list_stop(void);

#endif /* _SVC_LIST_H_ */
