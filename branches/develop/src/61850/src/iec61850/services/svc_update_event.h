/**
 *   \file svc_update_event.h
 *   \brief 事件触发更新ied数据
 */
#ifndef _SVC_UPDATE_EVENT_H_
#define _SVC_UPDATE_EVENT_H_

int svc_update_event_init(void);
int svc_update_event_exit(void);
int svc_update_event_run(void);
int svc_update_event_stop(void);

#endif /* _SVC_UPDATE_EVENT_H_ */
