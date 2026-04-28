/**
 *   \file sem.h
 *   \brief 信号量
 */
#ifndef _SEMPH_H_
#define _SEMPH_H_

#include <semaphore.h>
#include <sys/sem.h>
#include <stdbool.h>
#include "typedef.h"

typedef struct _SEMPH_DATA
{
	int semph_id;

}SEMPH_DATA;

/**
 *  \brief 创建信号量
 *  \param key
 *  \param mode
 *  \return 0 成功
 *  \return <0 失败
 */
int semph_create(SEMPH_DATA *pdata, key_t key, int mode);

/**
 *  \brief 打开信号量
 *  \param key
 *  \param mode
 *  \return 0 成功
 *  \return <0 失败
 */
int semph_open(SEMPH_DATA *pdata, key_t key, int mode);

/**
 *  \brief 删除信号量
 *  \param key
 */
void semph_remove(SEMPH_DATA *pdata);

/**
 *  \brief 锁信号量
 *  \return true 成功
 *  \return false 失败
 */
bool semph_lock(SEMPH_DATA *pdata);

/**
 *  \brief 解锁
 */
void semph_unlock(SEMPH_DATA *pdata);

/**
 *  \brief 设置值
 */
void semph_setval(SEMPH_DATA *pdata, int val);

/**
 *  \brief 获取值
 *  \return int 值
 */
int semph_getval(SEMPH_DATA *pdata);


#endif /* _SEMPH_H_ */
