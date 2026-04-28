#include "file_operation.h"
/* #include "log.h" */
/* #define FOP_PRINT_DEBUG(...) LOG_DEBUG(FOPLOG, __VA_ARGS__) */
#define FOP_PRINT_DEBUG(...) printf(__VA_ARGS__);

unsigned int file_operation_write(const char *pszFileName,
                                  unsigned char *pszBuf, int len)
{
    FILE *pFile = NULL;
    unsigned int num;

    if (NULL == pszFileName || NULL == pszBuf || len <= 0) {
        return 0;
    }

    /* Extract directory part to ensure it exists */
    const char *slash = strrchr(pszFileName, '/');
    if (slash != NULL && slash != pszFileName) {
        char pchDirPath[256];
        size_t dir_len = (size_t)(slash - pszFileName);
        if (dir_len >= sizeof(pchDirPath)) {
            return 0;
        }

        memcpy(pchDirPath, pszFileName, dir_len);
        pchDirPath[dir_len] = '\0';

        if (!file_operation_dir_exist(pchDirPath)) {
            printf("pchDirPath=%s\n", pchDirPath);
            file_operation_create_dir(pchDirPath);
        }
    }

    pFile = fopen(pszFileName, "a+");
    if (NULL == pFile) {
        perror("pszFileName");
        return 0;
    }

    num = fwrite(pszBuf, 1, len, pFile);
    fclose(pFile);

    return num;
}

unsigned int file_operation_read(const char *pszFileName, unsigned char *pszBuf,
                                 int len, unsigned int *uiReadpos)
{
    FILE *pFile = NULL;
    int num = 0;

    if (NULL == pszFileName || NULL == pszBuf || NULL == uiReadpos || len <= 0) {
        return 0;
    }
    pFile = fopen(pszFileName, "r");
    if (NULL == pFile) {
        perror("pszFileName");
        return 0;
    }

    if (-1 == fseek(pFile, *uiReadpos, SEEK_SET)) {
        fclose(pFile);
        return 0;
    }

    num = fread(pszBuf, 1, len, pFile);

    *uiReadpos += num;

    fclose(pFile);

    return num;
}

char *file_operation_read_malloc(char *file, int *len)
{
    FILE *f;
    char *data;

    if (file == NULL || len == NULL) {
        return NULL;
    }

    f = fopen(file, "rb");
    if (f == NULL) {
        fprintf(stderr, "open file %s failed\n", file);
        f = fopen(file, "wb+");
        if (f == NULL) {
            fprintf(stderr, "open file %s failed\n", file);
            return NULL;
        }
    }

    if (fseek(f, 0, SEEK_END) != 0) {
        fclose(f);
        return NULL;
    }

    long flen = ftell(f);
    if (flen < 0) {
        fclose(f);
        return NULL;
    }

    *len = (int)flen;
    data = (char *)malloc((*len + 1) * sizeof(char));
    /* log_debug(GLOG, "len=%d data=%p", *len, data); */
    if (NULL == data) {
        fclose(f);
        return NULL;
    }

    rewind(f);
    *len = fread(data, 1, *len, f);
    /* log_debug(GLOG, "len=%d", *len); */
    data[*len] = '\0';
    fclose(f);

    return data;
}

void file_operation_read_free(char **pdata)
{
    if (pdata != NULL && NULL != *pdata) {
        free(*pdata);
        *pdata = NULL;
    }
}

int file_operation_create_dir(const char *sPathName)
{
    char DirName[256] = "";
    int i, len;

    if (sPathName == NULL)
        return -1;

    size_t path_len = strlen(sPathName);
    if (path_len >= sizeof(DirName))
        return -1;

    /* If we need to append '/', ensure space for it and '\0'. */
    if (path_len > 0 && sPathName[path_len - 1] != '/') {
        if (path_len + 2 > sizeof(DirName)) {
            return -1;
        }
    }

    memcpy(DirName, sPathName, path_len);
    DirName[path_len] = '\0';

    len = strlen(DirName);
    if (len > 0 && DirName[len - 1] != '/') {
        DirName[len] = '/';
        DirName[len + 1] = '\0';
    }

    len = strlen(DirName);

    for (i = 1; i < len; i++) {
        if (DirName[i] == '/') {
            DirName[i] = 0;
            if (access(DirName, F_OK) != 0) {
                if (mkdir(DirName, 0755) == -1) {
                    perror("mkdir   error");
                    return -1;
                }
            }
            DirName[i] = '/';
        }
    }

    return 0;
}

unsigned long file_operation_get_size(const char *pchFileName)
{
    unsigned long filesize = 0;
    struct stat statbuff;

    if (stat(pchFileName, &statbuff) < 0) {
        return filesize;
    }
    else {
        filesize = statbuff.st_size;
    }

    return filesize;
}

static unsigned int uiTotalSize = 0;
static int CalDirSize(const char *pPath, const struct stat *s, int flagtype)
{
    printf("设备号:%lu 节点号:%lu 大小:%lu\n", s->st_dev, s->st_ino,
           s->st_size);
    uiTotalSize += s->st_size;
    return 0;
}

unsigned int GetDirSize(char *pszDirPath)
{
    int iRtn;
    uiTotalSize = 0;

    if (NULL == pszDirPath || access(pszDirPath, R_OK)) {
        printf("%s is not a dirpath\n", pszDirPath);
        return 0;
    }

    iRtn = ftw(pszDirPath, CalDirSize, 20);
    if (0 == iRtn) {
        // printf ( "path = %s totalsize= %d\n",
        // pszDirPath,  uiTotalSize );
    }

    return uiTotalSize;
} /* -----  end of function GetDirSize  ----- */

bool file_operation_exist(const char *pchFileName)
{
    struct stat statbuf;
    if (lstat(pchFileName, &statbuf) == 0) {
        return S_ISREG(statbuf.st_mode) != 0; //判断文件是否为常规文件
    }
    return false;
    // if( NULL == pchFileName )
    // {
    // return false;
    // }

    // if( 0 == access( pchFileName, F_OK ) )
    // {
    // return true;
    // }

    // return false;
} /* -----  end of function file_operation_exist  ----- */

bool file_operation_dir_exist(const char *pchDirPath)
{
    // if( NULL == pchDirPath )
    // {
    // return false;
    // }

    // if( NULL == opendir( pchDirPath ) )
    // {
    // return false;
    // }

    // return true;
    struct stat statbuf;
    if (lstat(pchDirPath, &statbuf) ==
        0) //lstat返回文件的信息，文件信息存放在stat结构中
    {
        return S_ISDIR(statbuf.st_mode) !=
               0; //S_ISDIR宏，判断文件类型是否为目录
    }

    return false;
}

//判断是否是特殊目录
bool IsSpecialDir(const char *pchDirPath)
{
    return strcmp(pchDirPath, ".") == 0 || strcmp(pchDirPath, "..") == 0;
}

/* Generate full file path safely. Return 0 on success, -1 on error. */
int file_operation_get_total_path(const char *path, const char *file_name,
                                  char *file_path, size_t file_path_len)
{
    if (path == NULL || file_name == NULL || file_path == NULL || file_path_len == 0) {
        return -1;
    }

    size_t path_len = strlen(path);
    int n;

    if (path_len == 0) {
        n = snprintf(file_path, file_path_len, "%s", file_name);
    }
    else if (path[path_len - 1] == '/') {
        n = snprintf(file_path, file_path_len, "%s%s", path, file_name);
    }
    else {
        n = snprintf(file_path, file_path_len, "%s/%s", path, file_name);
    }

    if (n < 0 || (size_t)n >= file_path_len) {
        file_path[file_path_len - 1] = '\0';
        return -1;
    }

    return 0;
}

bool file_operation_change_mode(char *pszFileName, int imode)
{
    if (NULL == pszFileName) {
        return false;
    }

    if (0 == chmod(pszFileName, imode)) {
        return true;
    }

    return false;
}

/*
 * ===  FUNCTION  ======================================================================
 *         Name:  ChangedirFileMode
 *  Description:  修改文件权限
 *		  Param:
 *		 Return:
 * =====================================================================================
 */
bool ChangedirFileMode(char *pchDirPath, int imode)
{
    DIR *dir;
    struct dirent *ptr;
    struct stat;
    char base[1024];

    if ((dir = opendir(pchDirPath)) == NULL) {
        perror("Open dir error...");
        return false;
    }

    while ((ptr = readdir(dir)) != NULL) {
        if (IsSpecialDir(ptr->d_name)) ///current dir OR parrent dir
            continue;
        else if (ptr->d_type == 8 || ///file
                 ptr->d_type == 10) ///link file
        {
            char chFileName[1024];
            int n = snprintf(chFileName, sizeof(chFileName), "%s/%s", pchDirPath,
                             ptr->d_name);
            if (n < 0 || (size_t)n >= sizeof(chFileName)) {
                closedir(dir);
                return false;
            }
            if (!file_operation_change_mode(chFileName, imode)) {
                closedir(dir);
                return false;
            }
        }
        else if (ptr->d_type == 4) ///dir
        {
            int n = snprintf(base, sizeof(base), "%s/%s", pchDirPath, ptr->d_name);
            if (n < 0 || (size_t)n >= sizeof(base)) {
                closedir(dir);
                return false;
            }
            if (!ChangedirFileMode(base, imode)) {
                closedir(dir);
                return false;
            }
        }
    }

    closedir(dir);
    return true;
} /* -----  end of function ChangedirFileMode  ----- */

/*
 * ===  FUNCTION  ======================================================================
 *         Name:  MoveDirFiles
 *  Description:  移动文件或目录  利用rename进行实现
 *		  Param:
 *		 Return:
 * =====================================================================================
 */
bool MoveDirFiles(char *pchSrcPath, char *pchDirPath)
{
    if (NULL == pchSrcPath || NULL == pchDirPath) {
        return false;
    }

    if (0 == rename(pchSrcPath, pchDirPath)) {
        return true;
    }

    return false;
} /* -----  end of function MoveDirFiles  ----- */

bool file_operation_is_change(char *pszFileName, struct stat *stat_info)
{
    struct stat st;
    if (stat(pszFileName, &st) != 0) {
        /* printf("%s error=1\n", __FUNCTION__); */
        return false; // 文件不存在或无法访问
    }

    if (st.st_mtime != stat_info->st_mtime ||
        st.st_size != stat_info->st_size) {
        printf("%s sucess\n", __FUNCTION__);
        memcpy(stat_info, &st, sizeof(struct stat));
        return true;
    }
    /* printf("%s error=2\n", __FUNCTION__); */
    return false;
}
