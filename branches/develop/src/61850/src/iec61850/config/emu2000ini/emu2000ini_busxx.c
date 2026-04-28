#include "ini.h"
#include "err_def.h"
#include "emu2000ini_busxx.h"
#include "file_operation.h"
#include "iec61850_macro.h"
#include "log.h"
#include "log_conf.h"

/* int g_emu2000ini_devtype_num = 0; */
/* int g_emu2000ini_devtype[32]; */
EMU2000_DEVTYPE_DATA g_emu2000_devtype_data = {
    .num = 0,
    .mask = 0,
    .devtype = { 0 },
};

/*  */
typedef struct _EMU2000_BUSXX_DEV_MODULEINX {
    int idx;
    char *name;
} EMU2000_BUSXX_DEV_MODULEINX;

EMU2000_BUSXX_DEV_MODULEINX gs_moduleidx[] = {
    { .idx = 30, .name = "pxr20" },
    { .idx = 31, .name = "pxr20" },
    { .idx = 32, .name = "pxr25" },
    { .idx = 33, .name = "pxr25" },
};

/*  */
typedef struct _EMU2000_BUSXX_DEV_INFO {
    int idx;

    IEC61850_CFG_DEV *pdev;
} EMU2000_BUSXX_DEV_INFO;

/**
 *  \brief busline 的读取处理
 *  \param
 *  \return 1成功
 */
static int busxx_dev_info_handle(void *user, const char *section,
                                 const char *name, const char *value)
{
    EMU2000_BUSXX_DEV_INFO *p = (EMU2000_BUSXX_DEV_INFO *)user;
    IEC61850_CFG_DEV *pdev = p->pdev;
    if (pdev == NULL) {
        return 0;
    }

    char dev[16];
    snprintf(dev, 16, "DEV%.3d", p->idx);

    if (EMU2000_INI_MATCH(dev, "module")) {
        int idx = atoi(value);
        int i;
        for (i = 0; i < sizeof(gs_moduleidx) / sizeof(gs_moduleidx[0]); i++) {
            EMU2000_BUSXX_DEV_MODULEINX *pmod = &gs_moduleidx[i];
            if (pmod->idx == idx) {
                free(pdev->template_name);
                pdev->template_name = strdup(pmod->name);
                if (NULL == pdev->template_name) {
                    log_error(CFGLOG, "busxx_dev_info_handle template_name strdup failed");
                    return 0;
                }
                break;
            }
        }

        return 0;
    }
    else if (EMU2000_INI_MATCH(dev, "serialno")) {
    }
    else if (EMU2000_INI_MATCH(dev, "addr")) {
        free(pdev->addr);
        pdev->addr = strdup(value);
        if (NULL == pdev->addr) {
            log_error(CFGLOG, "busxx_dev_info_handle addr strdup failed");
            return 0;
        }
    }
    else if (EMU2000_INI_MATCH(dev, "name")) {
        free(pdev->devname);
        /* pdev->devname = strdup(value); */
        pdev->devname = strdup("LD0");
        if (NULL == pdev->devname) {
            log_error(CFGLOG, "busxx_dev_info_handle devname strdup failed");
            return 0;
        }
    }
    else {
        return 0; /* unknown section/name, error */
    }

    return 1;

    return 1;
}

/**
 *  \brief busline 的读取处理
 *  \param
 *  \return 1成功
 */
static int busxx_dev_num_handle(void *user, const char *section,
                                const char *name, const char *value)
{
    int *pbsuline_num = (int *)user;
    if (EMU2000_INI_MATCH("DEVNUM", "NUM")) {
        *pbsuline_num = atoi(value);
    }
    else {
        return 0; /* unknown section/name, error */
    }

    return 1;
}

int emu2000_config_busxx_ini_parse(IEC61850_CONF *pcfg, char *name, int idx)
{
    char filename[256];
    snprintf(filename, 256, "%s/%s/Bus%.2d.ini", EMU2000_CONFIG_PATH, name,
             idx);
    log_info(CFGLOG, "emu2000_config_busxx_ini_parse filename=%s", filename);
    if (!file_operation_exist(filename)) {
        log_error(CFGLOG, "emu2000_config_busxx_ini_parse file %s not exist\n",
                  filename);
        return ERR_NOTEXIST;
    }

    int devnum = 0;
    int ret = ini_parse(filename, busxx_dev_num_handle, &devnum);
    if (ret <= 0) {
        log_error(CFGLOG, "emu2000_config_busxx_ini_parse ini_parse devnum "
                          "error\n");
        return ERR_NOCONFIG;
    }
    log_debug(CFGLOG, "devnum=%d\n", devnum);

    int i;
    for (i = 0; i < devnum; i++) {
        if (pcfg->devlist.num >= IEC61850_CFG_DEV_MAX) {
            log_error(CFGLOG, "emu2000_config_busxx_ini_parse devlist full %d",
                      pcfg->devlist.num);
            return ERR_OVERFLOW;
        }

        EMU2000_BUSXX_DEV_INFO devinfo;
        memset(&devinfo, 0, sizeof(devinfo));
        devinfo.idx = i + 1;
        devinfo.pdev = &pcfg->devlist.devs[pcfg->devlist.num];

        ret = ini_parse(filename, busxx_dev_info_handle, &devinfo);
        if (ret <= 0) {
            log_error(CFGLOG, "emu2000_config_busxx_ini_parse ini_parse "
                              "devinfo error\n");
            return ERR_NOCONFIG;
        }

        log_info(CFGLOG, "busline %d dev %d name=%s addr=%s template=%s\n", idx,
                 i + 1, devinfo.pdev->devname, devinfo.pdev->addr,
                 devinfo.pdev->template_name);
        char port[16];
        snprintf(port, 16, "%d", idx);
        if (NULL == devinfo.pdev->template_name) {
            log_error(CFGLOG,
                      "emu2000_config_busxx_ini_parse dev %d template "
                      "name is null\n",
                      i);
            return ERR_NOCONFIG;
        }
        free(devinfo.pdev->port);
        devinfo.pdev->port = strdup(port);
        if (devinfo.pdev->port == NULL) {
            log_error(CFGLOG,
                      "emu2000_config_busxx_ini_parse dev %d port strdup failed\n",
                      i);
            return ERR_MEM;
        }
        free(devinfo.pdev->type);
        devinfo.pdev->type = strdup(DEV_TYPE_METER);
        if (devinfo.pdev->type == NULL) {
            log_error(CFGLOG,
                      "emu2000_config_busxx_ini_parse dev %d type strdup failed\n",
                      i);
            return ERR_MEM;
        }
        pcfg->devlist.num++;
        log_debug(CFGLOG, "dev %d name=%s addr=%s template=%s\n", i,
                  devinfo.pdev->devname, devinfo.pdev->addr,
                  devinfo.pdev->template_name);
    }

    return ERR_OK;
}
