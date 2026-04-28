/**
 *   \file svc_61850_server.h
 *   \brief 61850服务器服务
 */
#ifndef _SVC_61850_SERVER_H_
#define _SVC_61850_SERVER_H_

int svc_61850_server_init(void);
int svc_61850_server_exit(void);
int svc_61850_server_run(void);
int svc_61850_server_stop(void);

#endif /* _SVC_61850_SERVER_H_ */
