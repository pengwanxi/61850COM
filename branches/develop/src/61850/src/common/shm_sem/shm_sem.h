/**
 *   \file shm_sem.h
 *   \brief 共享内存及信号量
 */
#ifndef _SHM_SEM_H_
#define _SHM_SEM_H_

#include "sharem.h"
#include "semph.h"

/* 数据 */
typedef struct _SHM_SEM_DATA
{
	SHAREM_DATA sharem_data;
	SEMPH_DATA semph_data;

	int key;

	int size;
	char *name;
	void *pdata;
	int nattach;
	/* bool binit; */

	/* F_OPS *pfops; */
}SHM_SEM_DATA;

/**
 *  \brief 创建共享内存和信号
 *  \param key
 *  \param size
 *  \param mode
 *  \return 0 成功
 *  \return <0 失败
 */
int shm_sem_create(SHM_SEM_DATA *pdata, key_t key, int size, int mode);

/**
 *  \brief 打开共享内存和信号
 *  \param key
 *  \param size
 *  \param mode
 *  \return 0 成功
 *  \return <0 失败
 */
int shm_sem_open(SHM_SEM_DATA *pdata, key_t key, int size, int mode);

/**
 *  \brief 打开共享内存和信号
 *  \param key
 *  \param size
 *  \param mode
 *  \return 0 成功
 *  \return <0 失败
 */
void shm_sem_remove(SHM_SEM_DATA *pdata);

/**
 *  \brief 挂接共享内存和信号
 *  \return !NULL 成功
 *  \return NULL 失败
 */
void* shm_sem_attach(SHM_SEM_DATA *pdata);

/**
 *  \brief 断开共享内存和信号
 */
void shm_sem_deattach(SHM_SEM_DATA *pdata);

/**
 *  \brief 删除共享内存和信号
 */
void shm_sem_deattach(SHM_SEM_DATA *pdata);

/**
 *  \brief 初始化共享内存和信号值
 */
void shm_sem_init_var(SHM_SEM_DATA *pdata);

/**
 *  \brief 锁信号量
 *  \return true 成功
 *  \return false 失败
 */
bool shm_sem_lock(SHM_SEM_DATA *pdata);

/**
 *  \brief 解锁
 */
void shm_sem_unlock(SHM_SEM_DATA *pdata);

/**
 *  \brief 设置值
 */
void shm_sem_set_semval(SHM_SEM_DATA *pdata, int val);

/**
 *  \brief 获取值
 *  \return int 值
 */
int shm_sem_get_semval(SHM_SEM_DATA *pdata);




#endif /* _SHM_SEM_H_ */
