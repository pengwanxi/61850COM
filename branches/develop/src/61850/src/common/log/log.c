#include <dirent.h>

#include "log.h"
#include "log_conf.h"

/* #include "file_operation.h" */

/* static bool gs_is_log_init = false; */
static int gs_log_fd = -1;
static char gs_conf[256] = LOGCFGPATH "/zlog.conf";

LOG_LIST gs_log_list;

void *log_init(const char *name)
{
    if (gs_log_fd < 0) {
        int fd = zlog_init(gs_conf);
        if (fd < 0) {
            printf("%s %s\n", __FUNCTION__, gs_conf);
            /* exit(-1); */
            return NULL;
        }
        gs_log_fd = fd;
    }

    return zlog_get_category(name);
}

void log_list_init(char *name, LOG_LIST list)
{
    int i;
    memset(&gs_log_list, 0, sizeof(LOG_LIST));

    if (NULL != name) {
        snprintf(gs_conf, sizeof(gs_conf), "%s/%s.conf", LOGCFGPATH, name);
    }

    for (i = 0; i < LOG_LIST_NUM; i++) {
        LOG_LIST_UNIT *p = &list.unit[i];
        /* printf("i=%d p=%p\n", i, p); */
        if (p->index <= 0 || strlen(p->name) < 1) {
            continue;
        }

        void *fd = log_init(p->name);
        if (fd) {
            p->fd = fd;
        }
        else {
            printf("%s init error\n", p->name);
        }
        /* printf("index=%d name=%s fd=%p ok\n", p->index, p->name, p->fd); */
    }
    memcpy(&gs_log_list, &list, sizeof(LOG_LIST));

    for (i = 1; i <= LOG_LIST_NUM; i++) {
        LOG_LIST_UNIT *p = &gs_log_list.unit[i-1];
        if (p->index <= 0 || strlen(p->name) < 1) {
            continue;
        }
        log_info(p->index, "%s log begin", p->name);
    }

    printf("%s ok\n", __FUNCTION__);
}

LOG_LIST_UNIT *log_list_unit(int index)
{
    /* printf("index=%d\n", index); */
    if (index > LOG_LIST_NUM || index <= 0) {
        return NULL;
    }

    if (gs_log_list.unit[index - 1].fd == NULL ||
        strlen(gs_log_list.unit[index - 1].name) < 2) {
        return NULL;
    }

    /* printf("p=%p %p\n", &gs_log_list.unit[index - 1], gs_log_list.unit[index - 1].fd); */
    return &gs_log_list.unit[index - 1];
}

void *log_list_fd(int index)
{
    LOG_LIST_UNIT *punit = log_list_unit(index);
    if (NULL == punit) {
        return gs_log_list.unit[0].fd;
    }

    /* printf("%d p=%p %p\n",index, &gs_log_list.unit[index - 1], gs_log_list.unit[index - 1].fd); */
    return punit->fd;
}

int log_list_cname(int index, char *name, int level)
{
    LOG_LIST_UNIT *punit = log_list_unit(index);
    if (NULL == punit) {
        return -1;
    }

    if (NULL == name) {
        return -2;
    }

    int i;
    for (i = 0; i < punit->cnum && i < LOG_SUB_ARRAY_NUM; i++) {
        if (level >= punit->cname[i].level) {
            if (0 == strcmp(punit->cname[i].name, name)) {
                return 2;
            }
        }
    }
    /* printf("p=%p %p\n", &gs_log_list.unit[index - 1], gs_log_list.unit[index - 1].fd); */
    return 0;
}

void log_exit()
{
    zlog_fini();
}

void log_name_frame(int l, int level, char *file, int line, char *name,
                    unsigned char *buf, int len, const char *fmt, ...)
{
    if (log_list_cname(l, name, level) <= 1) {
        return;
    }

    unsigned char pbuf[FRAME_LEN * 3 + 1] = "";
    char prefix[PREFIX_LEN] = "";
    char *ptr = (char *)pbuf;

    if (len > FRAME_LEN) {
        int size = len * 3 + 1;
        ptr = (char *)malloc(size);
        if (ptr == NULL) {
            return;
        }
        memset(ptr, 0, size);
    }

    int i;
    for (i = 0; i < len; i++) {
        /* 3 chars plus terminator */
        snprintf(&ptr[3 * i], 4, "%.2x ", (unsigned char)buf[i]);
    }

    va_list args;
    va_start(args, fmt);
    vsnprintf(prefix, PREFIX_LEN, fmt, args);
    va_end(args);

    log_level_buf(l, level, "[%s:%d:%s] %s:len=%.5d:%s", file, line, name,
                  prefix, len, ptr);

    if (len > FRAME_LEN) {
        free(ptr);
    }
}

void log_name_buf(int l, int level, char *file, int line, char *name,
                  const char *fmt, ...)
{
    if (log_list_cname(l, name, level) <= 1) {
        return;
    }
    char prefix[PREFIX_LEN] = "";

    va_list args;
    va_start(args, fmt);
    vsnprintf(prefix, PREFIX_LEN, fmt, args);
    va_end(args);

    log_level_buf(l, level, "[%s:%d:%s] %s", file, line, name, prefix);
}

void log_frame(int l, int level, char *file, int line, unsigned char *buf,
               int len, const char *fmt, ...)
{
    unsigned char pbuf[FRAME_LEN * 3 + 1] = "";
    char prefix[PREFIX_LEN] = "";
    char *ptr = (char *)pbuf;
    if (len > FRAME_LEN) {
        int size = len * 3 + 1;
        ptr = (char *)malloc(size);
        if (ptr == NULL) {
            return;
        }
        memset(ptr, 0, size);
    }

    int i;
    for (i = 0; i < len; i++) {
        /* 3 chars plus terminator */
        snprintf(&ptr[3 * i], 4, "%.2x ", (unsigned char)buf[i]);
    }

    va_list args;
    va_start(args, fmt);
    vsnprintf(prefix, PREFIX_LEN, fmt, args);
    va_end(args);

    log_level_buf(l, level, "[%s:%d] %s:len=%.5d:%s", file, line, prefix, len,
                  ptr);

    if (len > FRAME_LEN) {
        free(ptr);
    }
}

int log_set_cname(int index, int num, LOG_LIST_UNIT_CLASS *cname)
{
    LOG_LIST_UNIT *punit = log_list_unit(index);
    if (NULL == punit) {
        return -1;
    }

    if (num <= 0 || NULL == cname) {
        return -2;
    }

    int i;
    for (i = 0; i < num && i < LOG_SUB_ARRAY_NUM; i++) {
        memcpy(&punit->cname[i], &cname[i], sizeof(LOG_LIST_UNIT_CLASS));
    }

    punit->cnum = (num > LOG_SUB_ARRAY_NUM) ? LOG_SUB_ARRAY_NUM : num;

    return 0;
}

int log_find_cname(int index, char *cname)
{
    if (NULL == cname) {
        return -2;
    }

    LOG_LIST_UNIT *punit = log_list_unit(index);
    if (NULL == punit) {
        return -1;
    }

    if (punit->cnum <= 0) {
        return 0;
    }

    int i;
    for (i = 0; i < punit->cnum; i++) {
        if (0 == strcmp(punit->cname[i].name, cname)) {
            return 1;
        }
    }

    return 0;
}
