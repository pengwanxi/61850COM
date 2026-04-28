#include "ini.h"
#include "err_def.h"
#include "emu2000ini_busline.h"
#include "emu2000ini_busxx.h"
#include "iec61850_template_json.h"
#include "iec61850_macro.h"
#include "log.h"
#include "log_conf.h"

static char EMU2000_VALID_DIRS[][64] = {
    "ModBusMaster",
};

/*  */
typedef struct _EMU2000_BUSLINE_INFO {
    int idx;
    int pause;
    char para[128];
} EMU2000_BUSLINE_INFO;

/**
 *  \brief busline 的读取处理
 *  \param
 *  \return 1成功
 */
static int busline_port_para_handle(void *user, const char *section,
                                    const char *name, const char *value)
{
    EMU2000_BUSLINE_INFO *p = (EMU2000_BUSLINE_INFO *)user;
    char para[16];
    snprintf(para, 16, "para%.2d", p->idx);

    if (EMU2000_INI_MATCH("PORT", para)) {
        strncpy(p->para, value, sizeof(p->para) - 1);
    }
    else {
        return 0; /* unknown section/name, error */
    }

    return 1;
}

/**
 *  \brief busline 的读取处理
 *  \param
 *  \return 1成功
 */
static int busline_port_pause_handle(void *user, const char *section,
                                     const char *name, const char *value)
{
    EMU2000_BUSLINE_INFO *p = (EMU2000_BUSLINE_INFO *)user;
    char port[16];
    snprintf(port, 16, "PORT%.2d", p->idx);

    if (EMU2000_INI_MATCH("PORT", port)) {
        p->pause = 0;
        if (0 == strcmp(value, "PAUSE")) {
            p->pause = 1;
        }
    }
    else {
        return 0; /* unknown section/name, error */
    }

    return 1;
}

/**
 *  \brief busline 的读取处理
 *  \param
 *  \return 1成功
 */
static int busline_num_handle(void *user, const char *section, const char *name,
                              const char *value)
{
    int *pbsuline_num = (int *)user;
    if (EMU2000_INI_MATCH("LINE-NUM", "NUM")) {
        *pbsuline_num = atoi(value);
    }
    else {
        return 0; /* unknown section/name, error */
    }

    return 1;
}

/**
 *  \brief busline 的读取处理
 *  \param
 *  \return 1成功
 */
static int busline_project_handle(void *user, const char *section,
                                  const char *name, const char *value)
{
    IEC61850_CFG_DEV *pdev = (IEC61850_CFG_DEV *)user;
    if (EMU2000_INI_MATCH("PROJECT", "name")) {
        pdev->devname = strdup(value);
    }
    else {
        return 0; /* unknown section/name, error */
    }

    return 1;
}

int emu2000_config_busline_ini_parse(IEC61850_CONF *pcfg)
{
    int busline_num = 0;
    log_info(CFGLOG, "emu2000_config_busline_ini_parse %s start\n",
             EMU2000_BUSLINE_INI_NAME);
    int ret =
        ini_parse(EMU2000_BUSLINE_INI_NAME, busline_num_handle, &busline_num);
    if (ret <= 0) {
        log_error(CFGLOG, "emu2000_config_busline_ini_parse ini_parse "
                          "busline_num error\n");
        return ERR_NOCONFIG;
    }
    log_debug(CFGLOG, "busline_num=%d\n", busline_num);

    int i;
    for (i = 0; i < busline_num; i++) {
        EMU2000_BUSLINE_INFO info;
        memset(&info, 0, sizeof(EMU2000_BUSLINE_INFO));
        info.idx = i + 1;

        ret = ini_parse(EMU2000_BUSLINE_INI_NAME, busline_port_pause_handle,
                        &info);
        if (ret <= 0) {
            log_error(CFGLOG, "emu2000_config_busline_ini_parse ini_parse port "
                              "pause error\n");
            return ERR_NOCONFIG;
        }
        log_debug(CFGLOG, "busline %d pause=%d\n", info.idx, info.pause);

        if (info.pause) {
            log_info(CFGLOG, "busline %d is paused\n", info.idx);
            continue;
        }

        ret = ini_parse(EMU2000_BUSLINE_INI_NAME, busline_port_para_handle,
                        &info);
        if (ret <= 0) {
            log_error(CFGLOG, "emu2000_config_busline_ini_parse ini_parse port "
                              "para error\n");
            return ERR_NOCONFIG;
        }
        log_debug(CFGLOG, "busline %d para=%s\n", info.idx, info.para);

        int j;
        for (j = 0;
             j < sizeof(EMU2000_VALID_DIRS) / sizeof(EMU2000_VALID_DIRS[0]);
             j++) {
            if (strstr(info.para, EMU2000_VALID_DIRS[j]) != NULL) {
                ret = emu2000_config_busxx_ini_parse(
                    pcfg, EMU2000_VALID_DIRS[j], info.idx);
                if (ret != ERR_OK) {
                    log_error(CFGLOG,
                              "emu2000_config_busline_ini_parse "
                              "emu2000_config_busxx_ini_parse %s busline %d "
                              "error\n",
                              EMU2000_VALID_DIRS[j], info.idx);
                    return ret;
                }
                log_info(CFGLOG, "busline %d para=%s matched valid dir %s\n",
                         info.idx, info.para, EMU2000_VALID_DIRS[j]);
            }
        }

        log_info(CFGLOG, "busline %d para=%s\n", info.idx, info.para);
    }

    /* 手动增加ied 模型 */
    if (pcfg->devlist.num >= IEC61850_CFG_DEV_MAX) {
        log_error(CFGLOG, "emu2000_config_busline_ini_parse devlist full %d",
                  pcfg->devlist.num);
        return ERR_OVERFLOW;
    }
    IEC61850_CFG_DEV *pdev = &pcfg->devlist.devs[pcfg->devlist.num++];
    ret = ini_parse(EMU2000_BUSLINE_INI_NAME, busline_project_handle, pdev);
    if (ret <= 0) {
        log_error(CFGLOG, "emu2000_config_busline_ini_parse ini_parse project "
                          "error\n");
        free(pdev->devname);
        pdev->devname = strdup("ied");
    }

    if (NULL == pdev->devname) {
        log_error(CFGLOG, "emu2000_config_busline_ini_parse devname strdup failed\n");
        return ERR_MEM;
    }

    free(pdev->port);
    pdev->port = strdup("1");
    if (NULL == pdev->port) {
        log_error(CFGLOG, "emu2000_config_busline_ini_parse port strdup failed\n");
        return ERR_MEM;
    }

    free(pdev->addr);
    pdev->addr = strdup("1");
    if (NULL == pdev->addr) {
        log_error(CFGLOG, "emu2000_config_busline_ini_parse addr strdup failed\n");
        return ERR_MEM;
    }

    free(pdev->template_name);
    pdev->template_name = strdup("ied");
    if (NULL == pdev->template_name) {
        log_error(CFGLOG, "emu2000_config_busline_ini_parse template_name strdup failed\n");
        return ERR_MEM;
    }

    free(pdev->type);
    pdev->type = strdup(DEV_TYPE_IED);
    if (NULL == pdev->type) {
        log_error(CFGLOG, "emu2000_config_busline_ini_parse type strdup failed\n");
        return ERR_MEM;
    }
    log_debug(CFGLOG, "dev %d name=%s port=%s addr=%s template=%s\n", i,
              pdev->devname, pdev->port, pdev->addr, pdev->template_name);

    for (i = 0; i < pcfg->devlist.num; i++) {
        IEC61850_CFG_DEV *pdev = &pcfg->devlist.devs[i];
        log_debug(CFGLOG, "dev %d name=%s port=%s addr=%s template=%s\n", i,
                  pdev->devname, pdev->port, pdev->addr, pdev->template_name);

        char filename[128];
        snprintf(filename, sizeof(filename), "%s/template/%s.json", CONFIG_PATH,
                 pdev->template_name);
        ret = iec61850_config_template_json(pcfg, filename);
        if (ret != ERR_OK) {
            log_error(CFGLOG,
                      "emu2000_config_busline_ini_parse "
                      "iec61850_config_template_json %s error\n",
                      filename);
            return ret;
        }
    }

    return ERR_OK;
}
