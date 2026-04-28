#include <stdio.h>

#include "err_def.h"
#include "dev_list.h"
#include "dev_data.h"

#include "dev_ied.h"
#include "dev_PXR20.h"

#include "iec61850_conf.h"
#include "log.h"
#include "log_conf.h"
extern IEC61850_CONF *g_pcfg;

DEV_DATA gs_devs[DEVS_MAX];

DEV_DATA *dev_list_ied_data(void)
{
    if (g_pcfg == NULL) {
        log_error(DEVLOG, "dev_list_init g_pcfg is NULL");
        return NULL;
    }
    int num = g_pcfg->devlist.num > DEVS_MAX ? DEVS_MAX : g_pcfg->devlist.num;

    for (int i = 0; i < num; i++) {
        DEV_DATA *pdev = &gs_devs[i];
        if (pdev->proto == NULL) {
            continue;
        }
        if (strcmp(pdev->proto, TEMPLATE_PROTO_LOCAL_IEC61850) == 0) {
            return pdev;
        }
    }

    return NULL;
}

DEV_DATA *dev_list_data(char *name)
{
    if (g_pcfg == NULL) {
        log_error(DEVLOG, "dev_list_init g_pcfg is NULL");
        return NULL;
    }
    int num = g_pcfg->devlist.num > DEVS_MAX ? DEVS_MAX : g_pcfg->devlist.num;

    for (int i = 0; i < num; i++) {
        DEV_DATA *pdev = &gs_devs[i];
        if (pdev->devname == NULL) {
            continue;
        }
        if (strcmp(pdev->devname, name) == 0) {
            return pdev;
        }
    }

    return NULL;
}

DEV_DATA *dev_list_data_byguid(char *guid)
{
    if (g_pcfg == NULL) {
        log_error(DEVLOG, "dev_list_init g_pcfg is NULL");
        return NULL;
    }
    int num = g_pcfg->devlist.num > DEVS_MAX ? DEVS_MAX : g_pcfg->devlist.num;

    for (int i = 0; i < num; i++) {
        DEV_DATA *pdev = &gs_devs[i];
        if (pdev->devname == NULL) {
            continue;
        }
        if (strcmp(pdev->guid, guid) == 0) {
            return pdev;
        }
    }

    return NULL;
}

void *dev_list_private_data(char *name)
{
    DEV_DATA *pdev = dev_list_data(name);
    if (pdev == NULL) {
        return NULL;
    }

    return pdev->priv_data;
}

int dev_list_init(void)
{
    if (g_pcfg == NULL) {
        log_error(DEVLOG, "dev_list_init g_pcfg is NULL\n");
        return ERR_PNULL;
    }
    int ret = 0;

    /* 装置模型初始化 */
    log_info(DEVLOG, "dev_list_init start dev_init num=%d\n",
             g_pcfg->devlist.num);
    for (int i = 0; i < g_pcfg->devlist.num; i++) {
        IEC61850_CFG_DEV *pdev_cfg = &g_pcfg->devlist.devs[i];
        log_info(DEVLOG, "dev_list_init register name=%s %s\n",
                 pdev_cfg->devname, pdev_cfg->template_name);

        if (0 == strcmp(pdev_cfg->template_name, "ied")) {
            ret = dev_ied_init();
            if (ret != ERR_OK) {
                log_error(DEVLOG, "dev_ied_init failed ret=%d(%s)\n", ret,
                          err_str(ret));
                return ret;
            }
            log_info(DEVLOG, "dev_ied_init ok\n");
        }

        if (0 == strcmp(pdev_cfg->template_name, "pxr20") ||
            0 == strcmp(pdev_cfg->template_name, "pxr25")) {
            ret = dev_pxr20_init();
            if (ret != ERR_OK) {
                log_error(DEVLOG, "dev_ied_init failed ret=%d(%s)\n", ret,
                          err_str(ret));
                return ret;
            }
        }
    }

    if (g_pcfg->devlist.num > DEVS_MAX) {
        log_error(DEVLOG, "dev_list_init devlist num=%d exceed max=%d",
                  g_pcfg->devlist.num, DEVS_MAX);
        return ERR_O_RANGE;
    }
    memset(gs_devs, 0, sizeof(gs_devs));

    /* 装置列表初始化 */
    log_info(DEVLOG, "dev_list_init start dev_list num=%d",
             g_pcfg->devlist.num);
    for (int i = 0; i < g_pcfg->devlist.num; i++) {
        IEC61850_CFG_DEV *pdev_cfg = &g_pcfg->devlist.devs[i];
        DEV_DATA *pdev = &gs_devs[i];
        log_info(DEVLOG, "dev_list_init register name=%s %s", pdev_cfg->devname,
                 pdev_cfg->template_name);
        int ret = dev_data_init(pdev, pdev_cfg->template_name);
        if (ret != ERR_OK) {
            log_error(DEVLOG, "dev_data_init failed name=%s ret=%d(%s)",
                      pdev_cfg->template_name, ret, err_str(ret));
            return ret;
        }

        pdev->devname = strdup(pdev_cfg->devname);
        pdev->port = strdup(pdev_cfg->port);
        pdev->addr = strdup(pdev_cfg->addr);
        pdev->ptemplate = pdev->ptemplate;

        log_info(DEVLOG,
                 "dev_list_init register devname=%s name=%s template_name=%s "
                 "proto=%s ok",
                 pdev->devname, pdev->name, pdev->ptemplate->name, pdev->proto);
    }

    return ERR_OK;
}

int dev_list_exit(void)
{
    /* 释放列表 */
    for (int i = 0; i < g_pcfg->devlist.num; i++) {
        DEV_DATA *pdev = &gs_devs[i];
        dev_data_exit(pdev);
    }

    dev_pxr20_exit();
    dev_ied_exit();

    return 0;
}
