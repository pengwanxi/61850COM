/**
 *   \file log_bak.h
 *   \brief log_bak
 */
#ifndef _LOG_BAK_H_
#define _LOG_BAK_H_

#define LOG_PATH_MAX_LEN (512)
#define     LOG_PACK_MAX    (10)   //允许保存的压缩包数量
#define     BACKUP_TYPE "ZIP"

void log_backup(char *path);

#endif /* _LOG_BAK_H_ */
