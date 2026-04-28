#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/time.h>
#include <string.h>
#include <time.h>

#include "iec61850_server.h"
#include "iec61850_config_file_parser.h"
#include "list.h"
#include "iec61850_conf.h"
#include "err_def.h"
#include "log.h"
#include "log_conf.h"
#include "svc_list.h"
#include "data.h"
#include "hal.h"
#include "outdata.h"

IEC61850_CONF *g_pcfg = NULL; // 初始化为全0

static LOG_LIST_UNIT gs_log_list_unit[] = {
    { .index = GLOG, .name = "glog", .fd = NULL },
    { .index = STARTUP, .name = "startup", .fd = NULL },
    { .index = CFGLOG, .name = "cfg", .fd = NULL },
    { .index = COMMLOG, .name = "comm", .fd = NULL },
    { .index = OUTLOG, .name = "out", .fd = NULL },
    { .index = PROLOG, .name = "protocol", .fd = NULL },
    { .index = DEVLOG, .name = "dev", .fd = NULL },
    { .index = DVALOG, .name = "deval", .fd = NULL },
    { .index = MSGLOG, .name = "msg", .fd = NULL },
    { .index = SVCLOG, .name = "svc", .fd = NULL },
};

static void show_version()
{
    printf("platform:%s version:%s.%s.%s compile time: %s %s\n", PLATFORM_NAME,
           VERSION_MAJOR, VERSION_MINOR, VERSION_MICRO, __DATE__, __TIME__);
}

int main(int argc, char *argv[])
{
    /* 打印开头 */
    time_t rawtime;
    const struct tm *ptminfo;
    char tmp[1024] = { 0 };
    time(&rawtime);
    ptminfo = localtime(&rawtime);
    snprintf(&tmp[strlen(tmp)], sizeof(tmp) - strlen(tmp),
             "current: %02d-%02d-%02d %02d:%02d:%02d\n",
             ptminfo->tm_year + 1900, ptminfo->tm_mon + 1, ptminfo->tm_mday,
             ptminfo->tm_hour, ptminfo->tm_min, ptminfo->tm_sec);

    printf("%s", tmp);
    show_version();

    /* 初始化日志 */
    LOG_LIST log_list;
    memset(&log_list, 0, sizeof(log_list));
    memcpy(&log_list.unit, &gs_log_list_unit, sizeof(gs_log_list_unit));
    log_list_init("iec61850", log_list);

    log_conf_init();

    log_info(GLOG, "%s", tmp);

    log_info(STARTUP, "%s platform:%s version:%s.%s.%s compile time: %s %s",
             tmp, PLATFORM_NAME, VERSION_MAJOR, VERSION_MINOR, VERSION_MICRO,
             __DATE__, __TIME__);

    /* 外部数据初始化 */
    int ret = outdata_init();
    if (ret != ERR_OK) {
        log_error(GLOG, "outdata_init failed ret=%d(%s)", ret, err_str(ret));
        goto err;
    }
    log_info(GLOG, "outdata_init ok");

    /* 点表读取 */
    IEC61850_CONF *pcfg = NULL; // 初始化为全0
    ret = iec61850_config_init("emu2000ini", &pcfg);
    if (ret != ERR_OK) {
        log_error(GLOG, "iec61850_config_init failed ret=%d(%s)", ret,
                  err_str(ret));
        goto err;
    }
    log_info(GLOG, "iec61850_config_init ok");

    /* pcfg->pmodel = model; */
    /* pcfg->pmodel_idx_list = pmodel_idx_list; */
    g_pcfg = pcfg;

    ret = hal_init();
    if (ret != ERR_OK) {
        log_error(GLOG, "hal_init failed ret=%d(%s)", ret, err_str(ret));
        goto err;
    }

    ret = data_init();
    if (ret != ERR_OK) {
        log_error(GLOG, "data_init failed ret=%d(%s)", ret, err_str(ret));
        goto err;
    }
    log_info(GLOG, "data_init ok");

    ret = svc_list_init();
    if (ret != ERR_OK) {
        log_error(GLOG, "svc_list_init failed ret=%d(%s)", ret, err_str(ret));
        goto err;
    }
    log_info(GLOG, "svc_list_init ok");

    /* 等待线程 */
    int running = 1;

    int stop_count = 0;
    while (running) {
        sleep(1);

        /* stop_count++; */
        /* if (stop_count >= 20) { */
        /*     svc_list_stop(); */
        /* } */
        /* else { */
            svc_list_run();
        /* } */
    }

err:
    if (pcfg) {
        iec61850_config_exit(pcfg);
        g_pcfg = NULL;
    }

    svc_list_exit();
    data_exit();
    outdata_exit();
    return 0;
}
