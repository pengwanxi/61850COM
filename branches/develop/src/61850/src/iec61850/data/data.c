#include "data.h"
#include "err_def.h"

#include "log_conf.h"

#include "model_list.h"
#include "dev_list.h"
#include "ied_data.h"
#include "outdata.h"

extern IEC61850_CONF *g_pcfg;

int data_init()
{
    int ret;

    /* 初始化数据 */
    /* ret = model_list_init(); */
    /* if (ret != ERR_OK) { */
    /*     log_error(GLOG, "model_list_init failed ret=%d(%s)", ret, err_str(ret)); */
    /*     goto err1; */
    /* } */
    /* log_info(GLOG, "model_list_init ok"); */
    /* ret = outdata_init(); */
    /* if (ret != ERR_OK) { */
    /*     log_error(GLOG, "outdata_init failed ret=%d(%s)", ret, err_str(ret)); */
    /*     goto err; */
    /* } */
    /* log_info(GLOG, "outdata_init ok"); */

    ret = dev_list_init();
    if (ret != ERR_OK) {
        log_error(GLOG, "dev_list_init failed ret=%d(%s)", ret, err_str(ret));
        goto err4;
    }
    log_info(GLOG, "dev_list_init ok");

    ret = ied_data_init();
    if (ret != ERR_OK) {
        log_error(GLOG, "ied_data_init failed ret=%d(%s)", ret, err_str(ret));
        goto err3;
    }
    log_info(GLOG, "ied_data_init ok");

    DEV_DATA *pied = dev_list_ied_data();
    if (pied == NULL) {
        log_error(SVCLOG, "ied failed");
        goto err3;
    }
    int i;
    for (i = 0; i < g_pcfg->devlist.num; i++) {
        IEC61850_CFG_DEV *pcdev = &g_pcfg->devlist.devs[i];
        if (DEV_TYPE_MATCH_METER(pcdev->type)) {
            ied_update_all_quality(pied->devname, pcdev->devname,
                                   QUALITY_VALIDITY_INVALID |
                                       QUALITY_DETAIL_FAILURE);
        }
    }

    return ERR_OK;

err3:
    dev_list_exit();
err4:
    /* outdata_exit(); */
    /* err2: */
    /* model_list_exit(); */
    /* err1: */
    /*     ied_data_exit(); */

err:
    return ret;
}

int data_exit()
{
    outdata_exit();
    dev_list_exit();
    /* model_list_exit(); */
    /* ied_data_exit(); */

    return ERR_OK;
}
