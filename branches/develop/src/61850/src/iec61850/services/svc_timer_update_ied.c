#include <pthread.h>
#include <string.h>
#include <unistd.h>

#include "err_def.h"
#include "log_conf.h"

#include "svc_timer_update_ied.h"
#include "dev_data.h"
#include "dev_ied.h"
#include "dev_list.h"
#include "outdata.h"

#include "iec61850_conf.h"
extern IEC61850_CONF *g_pcfg;

static int gs_svc_timer_update_ied_thread_run = 0;
static pthread_t gs_svc_timer_update_ied_thread_id = 0;

static void *svc_timer_update_ied_thread(void *arg)
{
    gs_svc_timer_update_ied_thread_run = 1;
    DEV_DATA *pied = dev_list_ied_data();
    if (pied == NULL) {
        log_error(SVCLOG, "svc_timer_update_ied_thread dev_list_data ied "
                          "failed");
        gs_svc_timer_update_ied_thread_run = 0;
        return NULL;
    }

    while (gs_svc_timer_update_ied_thread_run) {
        sleep(1);
        log_debug(SVCLOG, "update data");

        int i;
        for (i = 0; i < g_pcfg->devlist.num; i++) {
            IEC61850_CFG_DEV *pcdev = &g_pcfg->devlist.devs[i];
            if (DEV_TYPE_MATCH_METER(pcdev->type)) {
                log_debug(SVCLOG, "devname=%s", pcdev->devname);
                DEV_DATA *pdev = dev_list_data(pcdev->devname);
                if (pdev == NULL) {
                    log_warn(SVCLOG,
                             "svc_timer_update_ied_thread dev_list_data %s "
                             "failed",
                             pcdev->devname);
                    continue;
                }

                DEV_VAL_LIST val_list;
                memset(&val_list, 0, sizeof(DEV_VAL_LIST));
                val_list.type = DVLT_TEMPLATE;

                if (pdev->get_list_val == NULL) {
                    log_warn(SVCLOG,
                             "svc_timer_update_ied_thread pdev->get_list_val "
                             "is NULL %s",
                             pcdev->devname);
                    continue;
                }
                int ret = pdev->get_list_val(pdev, &val_list);
                if (ret != ERR_OK) {
                    log_error(SVCLOG,
                              "svc_timer_update_ied_thread get_list_val failed "
                              "ret=%d(%s)",
                              ret, err_str(ret));
                    continue;
                }
                log_debug(SVCLOG, "get_list_val num=%d", val_list.num);

                for (int j = 0; j < val_list.num; j++) {
                    pied->set_val(pied, &val_list.vals[j]);
                }

                if (val_list.vals != NULL) {
                    for (int j = 0; j < val_list.num; j++) {
                        DEV_VAL *pval = &val_list.vals[j];
                        if (pval->name != NULL) {
                            free(pval->name);
                            pval->name = NULL;
                        }
                        if (pval->refname != NULL) {
                            free(pval->refname);
                            pval->refname = NULL;
                        }

                        if (pval->ref_time!= NULL) {
                            free(pval->ref_time);
                            pval->ref_time= NULL;
                        }
                        if (pval->ref_q!= NULL) {
                            free(pval->ref_q);
                            pval->ref_q= NULL;
                        }

                        if (pval->type == DVT_STRING) {
                            if (pval->str.val != NULL) {
                                free(pval->str.val);
                                pval->str.val = NULL;
                            }
                        }
                    }
                    val_list.num = 0;
                    free(val_list.vals);
                    val_list.vals = NULL;
                }
            }
        }
    }
    gs_svc_timer_update_ied_thread_run = 0;
    return NULL;
}

int svc_timer_update_ied_init(void)
{
    return ERR_OK;
}

int svc_timer_update_ied_exit(void)
{
    gs_svc_timer_update_ied_thread_run = 0;
    if (gs_svc_timer_update_ied_thread_id != 0) {
        pthread_cancel(gs_svc_timer_update_ied_thread_id);
        gs_svc_timer_update_ied_thread_id = 0;
    }

    return ERR_OK;
}

int svc_timer_update_ied_stop(void)
{
    return svc_timer_update_ied_exit();
}
int svc_timer_update_ied_run(void)
{
    if (gs_svc_timer_update_ied_thread_id != 0) {
        log_info(SVCLOG, "svc_timer_update_ied_run thread already running");
        return ERR_OK;
    }

    if (gs_svc_timer_update_ied_thread_run == 1) {
        log_info(SVCLOG, "svc_timer_update_ied_run already running");
        return ERR_OK;
    }

    pthread_create(&gs_svc_timer_update_ied_thread_id, NULL,
                   svc_timer_update_ied_thread, NULL);
    pthread_detach(gs_svc_timer_update_ied_thread_id);
    return ERR_OK;
}
