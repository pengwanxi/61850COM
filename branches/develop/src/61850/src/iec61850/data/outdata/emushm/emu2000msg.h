/**
 *   \file emu2000msg.h
 *   \brief 消息机制
 */
#ifndef _EMU2000MSG_H_
#define _EMU2000MSG_H_

/**
 *  \brief 初始化
 *  \param
 *  \return ERR_OK 成功 其他失败
 */
int emu2000msg_init();
int emu2000msg_exit();

int emu2000msg_send(int type, char *data, int len);
int emu2000msg_recv(char *data, int len);

#endif /* _EMU2000MSG_H_ */
