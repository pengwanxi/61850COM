#include <pthread.h>
#include <string.h>
#include <unistd.h>

#include "err_def.h"
#include "log_conf.h"
#include "svc_devlist_info.h"
#include "dev_list.h"
#include "iec61850_conf.h"
#include "outdata.h"

extern IEC61850_CONF *g_pcfg;

static int gs_svc_devlist_info_thread_run = 0;
static pthread_t gs_svc_devlist_info_thread_id = 0;

static void *svc_devlist_info_thread(void *arg)
{
    gs_svc_devlist_info_thread_run = 1;

    int i;
    for (i = 0; i < g_pcfg->devlist.num; i++) {
        IEC61850_CFG_DEV *pdev = &g_pcfg->devlist.devs[i];

        /* 打印配置装置信息 */
        log_info(SVCLOG, "Device %d:", i + 1);
        log_info(SVCLOG, "  Port: %s", pdev->port);
        log_info(SVCLOG, "  Addr: %s", pdev->addr);
    }

    for (i = 0; i < g_pcfg->template_list.num; i++) {
        IEC61850_CFG_TEMPLATE *ptemplate = &g_pcfg->template_list.templates[i];
        /* 打印配置模板信息 */
        log_info(SVCLOG, "Template %d:", i + 1);
        log_info(SVCLOG, "  Name: %s", ptemplate->name);
        log_info(SVCLOG, "  Protocol: %s", ptemplate->proto);

        IEC61850_CFG_TEMPLATE_MODEL *pmodel = &ptemplate->model;
        log_info(SVCLOG, "  Model Name: %s", pmodel->model);
        log_info(SVCLOG, "  Model Description: %s", pmodel->desc);
        log_info(SVCLOG, "  Model ProType: %s", pmodel->proType);
        log_info(SVCLOG, "  Model ManuID: %s", pmodel->manuID);
        log_info(SVCLOG, "  Model IsReport: %s", pmodel->isreport);

        int j;
        for (j = 0; j < ptemplate->dataset_list.num; j++) {
            IEC61850_CFG_TEMPLATE_DATASET *pdataset =
                &ptemplate->dataset_list.dataset[j];
            log_info(SVCLOG, "    Dataset %d:", j + 1);
            log_info(SVCLOG, "      Name: %s", pdataset->name);
            if (pdataset->ratio != NULL) {
                log_info(SVCLOG, "      Ratio: %s", pdataset->ratio);
            }
            if (pdataset->data_type != NULL) {
                log_info(SVCLOG, "      Data Type: %s", pdataset->data_type);
            }
            if (pdataset->data_idx != NULL) {
                log_info(SVCLOG, "      Data Index: %s", pdataset->data_idx);
            }
        }
    }

    while (gs_svc_devlist_info_thread_run) {
        log_debug(SVCLOG, "Querying device list...");

        for (i = 0; i < g_pcfg->devlist.num; i++) {
            IEC61850_CFG_DEV *pcdev = &g_pcfg->devlist.devs[i];
            if (DEV_TYPE_MATCH_METER(pcdev->type)) {
                /* 获取装置 */
                DEV_DATA *pdev = dev_list_data(pcdev->devname);
                if (pdev == NULL) {
                    log_warn(SVCLOG,
                             "svc_devlist_info_thread dev_list_data %s failed",
                             pcdev->devname);
                    continue;
                }

                int ret = outval_get_guid(pdev);
                if (ret != ERR_OK) {
                    log_error(SVCLOG,
                              "svc_devlist_info_thread outval_get_guid failed ret=%d",
                              ret);
                    continue;
                }
                else {
                    log_info(SVCLOG, "Device GUID: %s", pdev->guid);
                }

                /* 获取装置状态 */
                ret = outval_get_commstate(pdev);
                if (ret != ERR_OK) {
                    log_error(SVCLOG,
                              "svc_devlist_info_thread outval_get_commstate failed ret=%d",
                              ret);
                    continue;
                }
                else {
                    log_info(SVCLOG, "Device CommState: %d", pdev->commstate);
                }
            }
        }

        // TODO: 这里添加实际的设备列表查询逻辑
        // 可以使用 dev_list_data() 等函数获取设备信息

        sleep(5); // 每5秒查询一次设备列表
    }

    gs_svc_devlist_info_thread_run = 0;
    return NULL;
}

int svc_devlist_info_init(void)
{
    return ERR_OK;
}

int svc_devlist_info_exit(void)
{
    gs_svc_devlist_info_thread_run = 0;
    if (gs_svc_devlist_info_thread_id != 0) {
        pthread_cancel(gs_svc_devlist_info_thread_id);
        gs_svc_devlist_info_thread_id = 0;
    }
    return ERR_OK;
}

int svc_devlist_info_stop(void)
{
    return svc_devlist_info_exit();
}

int svc_devlist_info_run(void)
{
    if (gs_svc_devlist_info_thread_id != 0) {
        log_info(SVCLOG, "svc_devlist_info_run thread already running");
        return ERR_OK;
    }

    if (gs_svc_devlist_info_thread_run == 1) {
        log_info(SVCLOG, "svc_devlist_info_run already running");
        return ERR_OK;
    }

    pthread_create(&gs_svc_devlist_info_thread_id, NULL,
                   svc_devlist_info_thread, NULL);
    pthread_detach(gs_svc_devlist_info_thread_id);
    return ERR_OK;
}
