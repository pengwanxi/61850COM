/**
 *   \file sharem.h
 *   \brief 共享内存
 */
#ifndef _SHAREM_H_
#define _SHAREM_H_
#include <stdio.h>

#include <sys/shm.h>

/* 共享内存 */
typedef struct _SHAREM_DATA {
	int sharem_id;
	int size;

	void *psharem;
} SHAREM_DATA;

/**
 *  \brief 创建共享内存
 *  \param key
 *  \param size
 *  \param mode
 *  \return 0 成功
 *  \return <0 失败
 */
int sharem_create(SHAREM_DATA *pdata, key_t key, int size, int mode);

/**
 *  \brief 打开共享内存
 *  \param key
 *  \param size
 *  \param mode
 *  \return 0 成功
 *  \return <0 失败
 */
int sharem_open(SHAREM_DATA *pdata, key_t key, int size, int mode);

/**
 *  \brief 打开共享内存
 *  \return 0 成功
 *  \return <0 失败
 */
void sharem_remove(SHAREM_DATA *pdata);

/**
 *  \brief 挂接共享内存
 *  \return !NULL 成功
 *  \return NULL 失败
 */
void* sharem_attach(SHAREM_DATA *pdata);

/**
 *  \brief 断开共享内存
 */
void sharem_deattach(SHAREM_DATA *pdata);

/**
 *  \brief 删除共享内存
 */
void sharem_deattach(SHAREM_DATA *pdata);

/**
 *  \brief 初始化共享内存值
 */
void sharem_init_var(SHAREM_DATA *pdata);




#endif /* _SHAREM_H_ */
