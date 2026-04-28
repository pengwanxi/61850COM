/**
 *   \file msg.h
 *   \brief 消息队列
 */
#ifndef _MSG_H_
#define _MSG_H_

#include <sys/msg.h>

/*  */
typedef struct _MSG_DATA
{
    long type;

	char *data;

}MSG_DATA;


/*  */
typedef struct _MSG
{
	int msg_handle;
    int key;

}MSG;

/**
 *  \brief 根据key 创建消息队列
 *
 *  \return 消息队列 handle,小于0 时为创建失败
 */
int msg_create(MSG *pmsg);

/**
 *  \brief 根据消息队列handle 销毁消息队列
 *
 *  \return
 */
void msg_destroy(MSG *pmsg);

/**
 *  \brief 根据key 打开消息队列
 *
 *  \return 消息队列 handle,小于0 时为创建失败
 */
int msg_open(MSG *pmsg);

/**
 *  \brief 根据消息队列handle 关闭消息队列
 *
 *  \return
 */
void msg_close(MSG *pmsg);

/**
 *  \brief 消息通道是否已经创建
 *  \param
 *  \return 0 已经创建，－1 未创建
 */
int msg_is_exist(MSG *pmsg);

/**
 *  \brief 发送进程消息
 *  \param msg 要发送的消息指针
 *  \return 成功为0 失败为－1
 */
int msg_send(MSG *pmsg, MSG_DATA *pmsg_data, int len);

/**
 *  \brief 接收进程消息
 *  \param type 要接收的消息类型
 *  \param msg 要接收的消息指针
 *  \return 成功为收到的字节数 失败为0
 */
ssize_t msg_recv(MSG *pmsg, MSG_DATA *pmsg_data, int max);


#endif /* _MSG_H_ */
