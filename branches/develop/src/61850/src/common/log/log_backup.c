#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <dirent.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/stat.h>
#include <limits.h>

#include "log_backup.h"

bool file_exist(const char *filepath)
{
    struct stat sta;

    if (stat(filepath, &sta) < 0)
        return false;

    if (S_ISREG(sta.st_mode))
        return true;

    return false;
}

int file_path_extract(const char *src, char *path)
{
    int len, frag;
    char *off;

    if (!src || !path)
        return -1;

    len = strlen(src);

    off = (char *)&src[len - 1];

    frag = 0;
    while ((off != &src[0]) && (*off != '/')) {
        off--;
        frag++;
    }

    strncpy(path, src, len - frag); //!这个函数不会自动在字符串后面势\0'
    path[len - frag] = '\0';
    return 0;
}

bool path_exist(const char *pathname)
{
    struct stat sta;

    if (stat(pathname, &sta) < 0)
        return false;

    if (S_ISDIR(sta.st_mode))
        return true;

    return false;
}
/**
*********************************************************************
* @brief:       从文件路径中提取文件名
* @param[in]：   src         完整文件路径
* @param[in]:   withSubfix  提取文件名是否带后缀
* @param[out]：  filename    提取的文件名

* @return：      void
*********************************************************************
*/
int file_name_extract(const char *src, bool withSubfix, char *filename)
{
    int len, cnt, i;
    char *off;
    char name[NAME_MAX + 1];
    char name2[NAME_MAX + 1];

    memset(name, 0x00, sizeof(name));

    if (!src || !filename)
        return -1;

    len = strlen(src);

    off = (char *)&src[len - 1];

    cnt = 0;
    while ((off != &src[0]) && (*off != '/')) {
        name[cnt++] = *off--;
    }

    memset(name2, 0x0, sizeof(name2));

    for (i = 0; i < cnt; i++) {
        name2[i] = name[cnt - 1 - i];
    }

    for (i = 0; i < (int)strlen(name2); i++) {
        if ((withSubfix == false) && (name2[i] == '.')) {
            break;
        }

        *filename++ = name2[i];
    }

    *filename = '\0';

    return 0;
}

/**
*********************************************************************
* @brief:       从文件路径中提取文件扩展名
* @param[in]：   src         完整文件路径
* @param[out]：  ext         提取的扩展名

* @return：      void
*********************************************************************
*/
static int file_extension_extract(const char *src, char *ext)
{
    int len, frag;
    char *off;

    if (!src || !ext)
        return -1;

    len = strlen(src);

    off = (char *)&src[len - 1];

    frag = 0;
    while ((off != &src[0]) && (*off != '/') && (*off != '.')) {
        off--;
        frag++;
    }

    if (*off != '.') {
        *ext = '\0';
        return 0;
    }

    strncpy(ext, off + 1, len - frag);
    ext[len - frag] = '\0';
    return 0;
}

/**
*********************************************************************
* @brief:       将.log.sub文件打包
* @param[in]：   filepath 文件路径
* @param[in]:   storeCnt 压缩保存的份数
* @return：      void
*********************************************************************
*/
static int log_tar(const char *filepath, int storeCnt)
{
    int i;
    char tar_path[LOG_PATH_MAX_LEN * 2]; //压缩包路径
    char tar_cmd[LOG_PATH_MAX_LEN * 4]; //压缩包命令
    char rename_cmd1[LOG_PATH_MAX_LEN * 2]; //更换前的文件名
    char rename_cmd2[LOG_PATH_MAX_LEN * 2]; //更换后的文件名

    char bFilePath[LOG_PATH_MAX_LEN]; //文件路径（不含文件名）
    char bFileName[NAME_MAX]; //文件名，不含扩展名
    char bFileNameWithPostfix[NAME_MAX]; //文件名，含扩展名
    char bDirPath[LOG_PATH_MAX_LEN]; //文件路径（不含文件名）

    memset(tar_cmd, 0x0, sizeof(tar_cmd));
    memset(bFilePath, 0x0, sizeof(bFilePath));
    memset(bFileName, 0x0, sizeof(bFileName));
    memset(bFileNameWithPostfix, 0x0, sizeof(bFileNameWithPostfix));
    memset(bDirPath, 0x0, sizeof(bDirPath));

    file_path_extract(filepath, bFilePath); //文件路径（不含文件名）
    file_name_extract(filepath, false, bFileName); //文件名，不含扩展名
    file_name_extract(filepath, true, bFileNameWithPostfix); //文件名，含扩展名

    //在当前路径建立以日志文件为名的临时日志目录
    snprintf(bDirPath, sizeof(bDirPath), "%s", bFilePath);

    memset(tar_cmd, 0x0, sizeof(tar_cmd));

    for (i = 0; i < storeCnt; i++) //确认能够压缩的压缩包文件
    {
        memset(tar_path, 0x0, sizeof(tar_path));
        snprintf(tar_path, sizeof(tar_path), "%s/%s.%d.log.gz", bDirPath,
                 bFileName, i);

        if (!file_exist(tar_path)) //文件不存在，则可以压缩
        {
            // 不要包含目录
            memset(tar_path, 0x0, sizeof(tar_path));
            snprintf(tar_path, sizeof(tar_path), "%s.%d.log.gz", bFileName, i);
            snprintf(tar_cmd, sizeof(tar_cmd), "cd %s; tar -cvzf %s %s", bDirPath,
                     tar_path, bFileNameWithPostfix);
            system(tar_cmd);
            memset(tar_path, 0x0, sizeof(tar_path));
            snprintf(tar_path, sizeof(tar_path), "%s/%s", bDirPath,
                     bFileNameWithPostfix);
            remove(tar_path);
            return 1;
        }
    }

    //执行到这里，表示没有空闲的压缩包，所以需要将最旧的删除，并且将其他的挨个改名
    memset(tar_path, 0x0, sizeof(tar_path));
    snprintf(tar_path, sizeof(tar_path), "%s/%s.%d.log.gz", bDirPath, bFileName,
             0);
    remove(tar_path); //删除第一个压缩包

    for (i = 1; i < storeCnt; i++) {
        memset(rename_cmd1, 0x0, sizeof(rename_cmd1));
        memset(rename_cmd2, 0x0, sizeof(rename_cmd2));
        snprintf(rename_cmd1, sizeof(rename_cmd1), "%s/%s.%d.log.gz", bDirPath,
                 bFileName, i); //改名前
        snprintf(rename_cmd2, sizeof(rename_cmd2), "%s/%s.%d.log.gz", bDirPath,
                 bFileName, i - 1); //改名后
        rename(rename_cmd1, rename_cmd2);
    }

    memset(tar_path, 0x0, sizeof(tar_path));
    snprintf(tar_path, sizeof(tar_path), "%s.%d.log.gz", bFileName,
             storeCnt - 1);
    snprintf(tar_cmd, sizeof(tar_cmd), "cd %s; tar -cvzf %s %s", bDirPath,
             tar_path, bFileNameWithPostfix);
    system(tar_cmd); //压缩日志文件
    memset(tar_path, 0x0, sizeof(tar_path));
    snprintf(tar_path, sizeof(tar_path), "%s/%s", bDirPath,
             bFileNameWithPostfix);
    remove(tar_path);

    return 1;
}

/**
*********************************************************************
* @brief:       将.log.sub文件打包
* @param[in]：   filepath 文件路径
* @param[in]:   storeCnt 压缩保存的份数
* @return：      void
*********************************************************************
*/
static int log_zip(const char *filepath, int storeCnt)
{
    int i;
    char tar_path[LOG_PATH_MAX_LEN * 2]; //压缩包路径
    char tar_cmd[LOG_PATH_MAX_LEN * 4]; //压缩包命令
    char rename_cmd1[LOG_PATH_MAX_LEN * 2]; //更换前的文件名
    char rename_cmd2[LOG_PATH_MAX_LEN * 2]; //更换后的文件名

    char bFilePath[LOG_PATH_MAX_LEN]; //文件路径（不含文件名）
    char bFileName[NAME_MAX]; //文件名，不含扩展名
    char bFileNameWithPostfix[NAME_MAX]; //文件名，含扩展名
    char bDirPath[LOG_PATH_MAX_LEN]; //文件路径（不含文件名）

    memset(tar_cmd, 0x0, sizeof(tar_cmd));
    memset(bFilePath, 0x0, sizeof(bFilePath));
    memset(bFileName, 0x0, sizeof(bFileName));
    memset(bFileNameWithPostfix, 0x0, sizeof(bFileNameWithPostfix));
    memset(bDirPath, 0x0, sizeof(bDirPath));

    file_path_extract(filepath, bFilePath); //文件路径（不含文件名）
    file_name_extract(filepath, false, bFileName); //文件名，不含扩展名
    file_name_extract(filepath, true, bFileNameWithPostfix); //文件名，含扩展名

    //在当前路径建立以日志文件为名的临时日志目录
    snprintf(bDirPath, sizeof(bDirPath), "%s", bFilePath);

    memset(tar_cmd, 0x0, sizeof(tar_cmd));

    for (i = 0; i < storeCnt; i++) //确认能够压缩的压缩包文件
    {
        memset(tar_path, 0x0, sizeof(tar_path));
        snprintf(tar_path, sizeof(tar_path), "%s/%s.%d.log.zip", bDirPath,
                 bFileName, i);

        if (!file_exist(tar_path)) //文件不存在，则可以压缩
        {
            // 不要包含目录
            memset(tar_path, 0x0, sizeof(tar_path));
            snprintf(tar_path, sizeof(tar_path), "%s.%d.log.zip", bFileName, i);
            snprintf(tar_cmd, sizeof(tar_cmd), "cd %s; zip %s %s", bDirPath,
                     tar_path, bFileNameWithPostfix);
            system(tar_cmd);
            memset(tar_path, 0x0, sizeof(tar_path));
            snprintf(tar_path, sizeof(tar_path), "%s/%s", bDirPath,
                     bFileNameWithPostfix);
            remove(tar_path);
            return 1;
        }
    }

    //执行到这里，表示没有空闲的压缩包，所以需要将最旧的删除，并且将其他的挨个改名
    memset(tar_path, 0x0, sizeof(tar_path));
    snprintf(tar_path, sizeof(tar_path), "%s/%s.%d.log.gz", bDirPath, bFileName,
             0);
    remove(tar_path); //删除第一个压缩包

    for (i = 1; i < storeCnt; i++) {
        memset(rename_cmd1, 0x0, sizeof(rename_cmd1));
        memset(rename_cmd2, 0x0, sizeof(rename_cmd2));
        snprintf(rename_cmd1, sizeof(rename_cmd1), "%s/%s.%d.log.zip", bDirPath,
                 bFileName, i); //改名前
        snprintf(rename_cmd2, sizeof(rename_cmd2), "%s/%s.%d.log.zip", bDirPath,
                 bFileName, i - 1); //改名后
        rename(rename_cmd1, rename_cmd2);
    }

    memset(tar_path, 0x0, sizeof(tar_path));
    snprintf(tar_path, sizeof(tar_path), "%s.%d.log.zip", bFileName,
             storeCnt - 1);
    snprintf(tar_cmd, sizeof(tar_cmd), "cd %s; zip %s %s", bDirPath, tar_path,
             bFileNameWithPostfix);
    system(tar_cmd); //压缩日志文件
    memset(tar_path, 0x0, sizeof(tar_path));
    snprintf(tar_path, sizeof(tar_path), "%s/%s", bDirPath,
             bFileNameWithPostfix);
    remove(tar_path);

    return 1;
}

/**
*********************************************************************
* @brief:       将.log.sub文件打包
* @param[in]：   NA
* @param[out]:  NA
* @return：      void
*********************************************************************
*/
void log_backup(char *path)
{
    //    int i = 0;
    DIR *dirp;
    struct dirent *entp;
    char subpath[LOG_PATH_MAX_LEN + 1];
    char subfix[128] = { 0 };

    //打开目录
    if ((dirp = opendir(path)) == NULL) {
        return;
    }

    while ((entp = readdir(dirp)) != NULL) {
        memset(subpath, 0, sizeof(subpath));
        memset(subfix, 0, sizeof(subfix));

        if ((strcmp(entp->d_name, ".") == 0) ||
            (strcmp(entp->d_name, "..") == 0)) {
            continue;
        }

        /* 只备份 .log.的文件 */
        if (NULL == strstr(entp->d_name, ".log."))
            continue;

        snprintf(subpath, sizeof(subpath), "%s/%s", path, entp->d_name);

        //获取文件后缀
        file_extension_extract(entp->d_name, subfix);

        if (0 == strcmp(BACKUP_TYPE, "ZIP")) {
            //打包后缀不为.log的文件
            if ((strlen(subfix) != 0) && (strcmp(subfix, "log") != 0) &&
                (strcmp(subfix, "zip") != 0)) {
                usleep(10000); //延时10ms 等待文件拷贝完成
                log_zip(subpath, LOG_PACK_MAX);
                sleep(1);
            }
        }
        else {
            //打包后缀不为.log的文件
            if ((strlen(subfix) != 0) && (strcmp(subfix, "log") != 0) &&
                (strcmp(subfix, "gz") != 0)) {
                usleep(10000); //延时10ms 等待文件拷贝完成
                log_tar(subpath, LOG_PACK_MAX);
                sleep(1);
            }
        }
    }

    closedir(dirp);

    return;
}
