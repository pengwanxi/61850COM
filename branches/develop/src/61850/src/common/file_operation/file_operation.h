/**
 *   \file file_operation.h
 *   \brief 一些文件的操作
 */
#ifndef _FILE_OPERATION_H_
#define _FILE_OPERATION_H_

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <dirent.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <ftw.h>
#include <sys/types.h>
#include <fcntl.h>
#include <string.h>
#include <dirent.h>
#include <stdlib.h>
#include <stdbool.h>

/**
 *  \brief 写文件
 *  \param 文件名称
 *  \param 数据
 *  \param 长度
 *  \return 写入长度
 */
unsigned int file_operation_write(const char *pszFileName,
                                  unsigned char *pszBuf, int len);
/**
 *  \brief 读文件
 *  \param 文件名称
 *  \param 数据
 *  \param 长度
 *  \param 位置
 *  \return 读出长度
 */
unsigned int file_operation_read(const char *pszFileName, unsigned char *pszBuf,
                                 int len, unsigned int *uiReadpos);

/**
 *  \brief 打开文件并将所有数据取出来
 *  \param file 文件名
 *  \param len 需要返回的数据长度
 *  \return 文件数据
 */
char *file_operation_read_malloc(char *file, int *len);

/**
 *  \brief 删除数据
 *  \param 已经malloc的数据
 *  \return void
 */
void file_operation_read_free(char **pdata);

/**
 *  \brief 创建文件夹
 *  \param 文件夹
 *  \return 0 成功
 *  \return -1 失败
 */
int file_operation_create_dir(const char *sPathName);

/**
 *  \brief 获取文件大小
 *  \param 文件
 *  \return 大小
 */
unsigned long file_operation_get_size(const char *pchFileName);

/**
 *  \brief 文件是否存在
 *  \param 文件名
 *  \return true
 */
bool file_operation_exist(const char *pchFileName);

/**
 *  \brief 文件夹是否存在
 *  \param 文件夹名
 *  \return true
 */
bool file_operation_dir_exist(const char *pchDirPath);

/**
 *  \brief 生成完整的文件路径
 *  \param 文件夹路径
 *  \param 文件名称（不含路径）
 *  \param 全路径
 *  \return void
 */
int file_operation_get_total_path(const char *path, const char *file_name,
                                  char *file_path, size_t file_path_len);

/**
 *  \brief 修改文件权限
 *  \param 文件名称
 *  \param 文件权限
 *  \return bool
 */
bool file_operation_change_mode(char *pszFileName, int imode);

/**
 *  \brief 修改文件权限
 *  \param 文件名称
 *  \param 文件权限
 *  \return bool
 */
bool file_operation_is_change(char *pszFileName, struct stat *stat);

#endif /* _FILE_OPERATION_H_ */
