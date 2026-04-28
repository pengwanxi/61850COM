#include <stdio.h>

#include "list.h"

#include "err_def.h"
#include "model_list.h"
#include "iec61850_conf.h"

#include "log_conf.h"

#include "model_ied.h"

extern IEC61850_CONF *g_pcfg;

int model_list_init(void)
{
    if (g_pcfg == NULL) {
        log_error(GLOG, "model_list_init g_pcfg is NULL");
        return ERR_PNULL;
    }

    /* /\* 模型初始化 *\/ */
    /* int ret = model_ied_init(); */
    /* if (ret != ERR_OK) { */
    /*     log_error(GLOG, "model_ied_init failed ret=%d(%s)", ret, err_str(ret)); */
    /*     return ret; */
    /* } */

    return ERR_OK;
}

int model_list_exit(void)
{
    /* model_ied_exit(); */
    return ERR_OK;
}
