#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>

#include "iec61850_conf.h"
#include "emu2000ini.h"
#include "err_def.h"

#include "log_conf.h"

#include "iec61850_config_json.h"

IEC61850_CFG_DEV *iec61850_config_dev(IEC61850_CONF *pcfg, char *name)
{
    if (NULL == pcfg || NULL == name) {
        return NULL;
    }

    for (int i = 0; i < pcfg->devlist.num; i++) {
        IEC61850_CFG_DEV *pdev = &pcfg->devlist.devs[i];
        if (0 == strcmp(pdev->devname, name)) {
            return pdev;
        }
    }

    return NULL;
}

IEC61850_CFG_TEMPLATE *iec61850_config_template(IEC61850_CONF *pcfg, char *name)
{
    if (NULL == pcfg || NULL == name) {
        return NULL;
    }

    for (int i = 0; i < pcfg->template_list.num; i++) {
        IEC61850_CFG_TEMPLATE *ptemplate = &pcfg->template_list.templates[i];
        if (0 == strcmp(ptemplate->name, name)) {
            return ptemplate;
        }
    }

    return NULL;
}

int iec61850_config_init(char *name, IEC61850_CONF **pcfg)
{
    if (pcfg == NULL || name == NULL) {
        return ERR_PNULL;
    }

    log_info(CFGLOG, "iec61850_config_init name=%s", name);

    *pcfg = (IEC61850_CONF *)malloc(sizeof(IEC61850_CONF));
    if (*pcfg == NULL) {
        return ERR_MEM;
    }
    memset(*pcfg, 0, sizeof(IEC61850_CONF));

    int ret = ERR_OK;
    if (0 == strcmp(name, "json")) {
        char *filename = CONFIG_PATH "iec61850.json";
        log_info(CFGLOG, "filename=%s", filename);
        ret = iec61850_config_json_parse(*pcfg, filename);
        if (ret != ERR_OK) {
            log_error(CFGLOG, "iec61850_config_json_parse failed ret=%d(%s)",
                      ret, err_str(ret));
            goto err;
        }
    }
    else if (0 == strcmp(name, "emu2000ini")) {
        ret = emu2000_config_ini_parse(*pcfg);
        if (ret != ERR_OK) {
            log_error(CFGLOG, "emu2000_config_ini_parse failed ret=%d(%s)",
                      ret, err_str(ret));
            goto err;
        }
    }
    else {
        ret = ERR_NOCONFIG;
        goto err;
    }

    return ERR_OK;

err:
    iec61850_config_exit(*pcfg);
    *pcfg = NULL;
    return ret;
}

int iec61850_config_exit(IEC61850_CONF *pcfg)
{
    if (pcfg == NULL) {
        return ERR_PNULL;
    }

    /* devlist */
    for (int i = 0; i < pcfg->devlist.num; i++) {
        IEC61850_CFG_DEV *pdev = &pcfg->devlist.devs[i];
        free(pdev->port);
        free(pdev->addr);
        free(pdev->devname);
        free(pdev->template_name);
        free(pdev->type);
        pdev->port = NULL;
        pdev->addr = NULL;
        pdev->devname = NULL;
        pdev->template_name = NULL;
        pdev->type = NULL;
    }
    pcfg->devlist.num = 0;

    /* template_list */
    for (int i = 0; i < pcfg->template_list.num; i++) {
        IEC61850_CFG_TEMPLATE *pt = &pcfg->template_list.templates[i];

        free(pt->name);
        free(pt->proto);
        free(pt->version);
        free(pt->desc);
        free(pt->ldname);
        pt->name = NULL;
        pt->proto = NULL;
        pt->version = NULL;
        pt->desc = NULL;
        pt->ldname = NULL;

        free(pt->model.model);
        free(pt->model.desc);
        free(pt->model.proType);
        free(pt->model.manuID);
        free(pt->model.isreport);
        pt->model.model = NULL;
        pt->model.desc = NULL;
        pt->model.proType = NULL;
        pt->model.manuID = NULL;
        pt->model.isreport = NULL;

        for (int j = 0; j < pt->dataset_list.num; j++) {
            IEC61850_CFG_TEMPLATE_DATASET *ds = &pt->dataset_list.dataset[j];
            free(ds->name);
            free(ds->name_desc);
            free(ds->ref_name);
            free(ds->db_name);
            free(ds->data_type);
            free(ds->data_idx);
            free(ds->data_desc);
            free(ds->ratio);
            ds->name = NULL;
            ds->name_desc = NULL;
            ds->ref_name = NULL;
            ds->db_name = NULL;
            ds->data_type = NULL;
            ds->data_idx = NULL;
            ds->data_desc = NULL;
            ds->ratio = NULL;
        }
        pt->dataset_list.num = 0;

        for (int j = 0; j < pt->event_list.num; j++) {
            IEC61850_CFG_TEMPLATE_EVENT *ev = &pt->event_list.event[j];
            free(ev->name);
            free(ev->ref_name);
            free(ev->ref_time);
            free(ev->yx_idx);
            free(ev->event_idx);
            ev->name = NULL;
            ev->ref_name = NULL;
            ev->ref_time = NULL;
            ev->yx_idx = NULL;
            ev->event_idx = NULL;
        }
        pt->event_list.num = 0;

        /* Clean up event_data_list */
        for (int j = 0; j < pt->event_data_list.num; j++) {
            IEC61850_CFG_TEMPLATE_EVENT_DATA *ed = &pt->event_data_list.data[j];
            free(ed->idx);
            free(ed->name);
            ed->idx = NULL;
            ed->name = NULL;
            
            for (int k = 0; k < ed->num; k++) {
                IEC61850_CFG_TEMPLATE_EVENT_DATA_ITEM *item = &ed->items[k];
                free(item->name);
                free(item->ref_name);
                free(item->idx);
                item->name = NULL;
                item->ref_name = NULL;
                item->idx = NULL;
            }
            ed->num = 0;
        }
        pt->event_data_list.num = 0;
    }
    pcfg->template_list.num = 0;

    free(pcfg->name);
    free(pcfg->version);
    pcfg->name = NULL;
    pcfg->version = NULL;

    free(pcfg);
    return ERR_OK;
}
