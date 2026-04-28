#include <string.h>

#include "iec61850_conf.h"
#include "err_def.h"
#include "list.h"
#include "iec61850_server.h"

#include "dev_data.h"
#include "dev_ied.h"
#include "ied_data.h"

#include "log_conf.h"

extern IEC61850_CONF *g_pcfg;

/*  */
typedef struct _DEV_IED_PRIVATE_DATA {
    list_t *pdataset_list;
    list_t *pmodel_idx_list;

} DEV_IED_PRIVATE_DATA;

static DEV_IED_PRIVATE_DATA gs_private_data;

/**
 *  \brief 设置数据
 *  \param pdev
 *  \param dval
 *  \return ERR_OK-成功 其他-失败
 */
int dev_ied_set_val(DEV_DATA *pdev, DEV_VAL *pval)
{
    if (pdev == NULL || pval == NULL) {
        return ERR_PNULL;
    }

    if (NULL == pval->refname) {
        log_warn(DEVLOG, "dev_ied_set_val name=%s null refname", pval->name);
        return ERR_PNULL;
    }

    char refname[512];
    snprintf(refname, sizeof(refname), "%s%s", pdev->devname, pval->refname);

    log_debug(DEVLOG, "dev_ied_set_val name=%s %s type=%d iedrefname=%s",
              pval->name, pval->refname, pval->type, refname);

    int ref_time_valid = 0;
    if (pval->ref_time) {
        char reftime[512];
        snprintf(reftime, sizeof(reftime), "%s%s", pdev->devname,
                 pval->ref_time);

        log_debug(DVALOG, "%s choice=%d\n", reftime, pval->t.choice);

        if (pval->t.choice == 1) {
            log_debug(DVALOG,
                      "dev_ied_set_val name=%s ref_time=%s acsi_time=%u.%u",
                      pval->name, reftime, pval->t.acsi_time.SecondSinceEpoch,
                      pval->t.acsi_time.FractionOfSecond);
            ied_update_utctime(
                reftime, (uint64_t)pval->t.acsi_time.SecondSinceEpoch * 1000 +
                             pval->t.acsi_time.FractionOfSecond / 1000000);
            ref_time_valid = 1;
        }
        else if (pval->t.choice == 2) {
            log_debug(DVALOG,
                      "dev_ied_set_val name=%s ref_time=%s utc_time=%llu",
                      pval->name, reftime, pval->t.utc_time);
            ied_update_utctime(reftime, pval->t.utc_time);
            ref_time_valid = 1;
        }
        else {
            log_warn(DEVLOG,
                     "dev_ied_set_val name=%s ref_time=%s invalid choice=%d",
                     pval->name, reftime, pval->t.choice);
        }
    }

    if (pval->ref_q) {
        char ref_q[512];
        snprintf(ref_q, sizeof(ref_q), "%s%s", pdev->devname, pval->ref_q);
        uint16_t valid = 0;

        if (pval->type == DVT_COMMSTATE_ERROR) {
            log_warn(DEVLOG,
                     "dev_ied_set_val name=%s ref_q=%s communication error, "
                     "cannot update quality",
                     pval->name, pval->ref_q);

            valid = QUALITY_VALIDITY_INVALID | QUALITY_DETAIL_OLD_DATA;
        }
        else if (ref_time_valid) {
            log_debug(DVALOG,
                      "dev_ied_set_val name=%s ref_q=%s update quality %s",
                      pval->name, pval->ref_q, pval->ref_q);

            valid = QUALITY_VALIDITY_GOOD;
        }
        else {
            log_warn(DEVLOG,
                     "dev_ied_set_val name=%s ref_q=%s invalid ref_time, "
                     "cannot update quality",
                     pval->name, pval->ref_q);

            valid = QUALITY_VALIDITY_INVALID | QUALITY_DETAIL_FAILURE;
        }

        log_debug(DVALOG, "%s valid=%d\n", ref_q, valid);
        ied_update_quality(ref_q, valid);
    }

    switch (pval->type) {
    case DVT_BOOLEAN: {
        log_debug(DVALOG, "dev_ied_set_val name=%s bval=%d", pval->refname,
                  pval->bval);
        return ied_update_bval(refname, pval->bval);
    } break;
    case DVT_FLOAT: {
        log_debug(DVALOG, "dev_ied_set_val name=%s fval=%f", pval->refname,
                  pval->fval);
        return ied_update_float(refname, pval->fval);
    } break;
    case DVT_INT:
    case DVT_DWORD: {
        log_debug(DVALOG, "dev_ied_set_val name=%s ival=%d", pval->refname,
                  pval->ival);
        return ied_update_ival(refname, pval->ival);
    }
    case DVT_DBPOS: {
        log_debug(DVALOG, "dev_ied_set_val name=%s dbpos=%d", pval->refname,
                  pval->ival);
        return ied_update_dbpos(refname, pval->ival);
    }
    case DVT_STRING: {
        log_debug(DVALOG, "dev_ied_set_val name=%s sval=%s", pval->refname,
                  pval->str.val);
        return ied_update_sval(refname, pval->str.val);
    }
    case DVT_UTCTIME: {
        log_debug(DVALOG, "dev_ied_set_val name=%s time=%llu", pval->refname,
                  pval->utc_time);
        return ied_update_utctime(refname, pval->utc_time);
    }
    default:
        log_warn(DEVLOG, "dev_ied_set_val name=%s type=%d not support", refname,
                 pval->type);
        break;
    }

    /* IedServer_updateFloatAttributeValue(g_pcfg->pied_server, */
    /*                                     model_idx->pattr, 12.34); */

    return ERR_OK;
}

int dev_ied_set_record(DEV_DATA *pdev, DEV_VAL *pval)
{
    if (pdev == NULL || pval == NULL) {
        return ERR_PNULL;
    }
    char refname[512];
    snprintf(refname, sizeof(refname), "%s%s", pdev->devname, pval->refname);

    log_debug(DEVLOG, "dev_ied_set_val name=%s %s type=%d iedrefname=%s",
              pval->name, pval->refname, pval->type, refname);

    switch (pval->type) {
    case DVT_EVENT: {
        DEV_VAL_EVENT *pevent = (DEV_VAL_EVENT *)pval->event;
        if (pevent == NULL) {
            log_warn(DEVLOG, "dev_ied_set_record name=%s invalid event",
                     pval->refname);
            return ERR_O_RANGE;
        }
        log_debug(DVALOG, "dev_ied_set_record name=%s time=%s q=%s bval=%d",
                  pevent->ref_name, pevent->ref_time, pevent->ref_q,
                  pevent->state);
        ied_update_bval(pevent->ref_name, pevent->state);
        ied_update_utctime(pevent->ref_time, pevent->utc_time);
        if (pevent->ref_q_valid) {
            log_debug(DVALOG, "dev_ied_set_record name=%s update quality %s",
                      pevent->ref_q, pevent->ref_q);
            ied_update_quality(pevent->ref_q, QUALITY_VALIDITY_GOOD);
        }

    } break;
    default:
        log_warn(DEVLOG, "dev_ied_set_record name=%s type=%d not support",
                 pval->refname, pval->type);
        break;
    }

    return ERR_OK;
}

DEV_DATA gs_dev_data_ied[] = {
    {
        .name = "ied",
        .proto = "iec61850",
        .init = 0,
        .set_val = dev_ied_set_val,
        .get_val = NULL,
        .set_record = dev_ied_set_record,
        .priv_data = &gs_private_data,
    },
    {
        .name = "ied",
        .proto = "modbus_tcp",
        .init = 0,
        .set_val = dev_ied_set_val,
        .set_record = dev_ied_set_record,
        .get_val = NULL,
        .priv_data = &gs_private_data,
    },
};

/**
 *  \brief match 装置数据
 *  \param name protocol
 *  \return NULL-失败 其他-成功
 */
static DEV_DATA *match_dev_ied(char *name, char *protocol)
{
    if (name == NULL) {
        return NULL;
    }

    for (int i = 0;
         i < (int)(sizeof(gs_dev_data_ied) / sizeof(gs_dev_data_ied[0])); i++) {
        DEV_DATA *pdev = &gs_dev_data_ied[i];
        if (strcmp(pdev->name, name) == 0 &&
            strcmp(pdev->proto, protocol) == 0) {
            return pdev;
        }
    }

    return NULL;
}

int dev_ied_init()
{
    if (g_pcfg == NULL) {
        return ERR_PNULL;
    }

    IEC61850_CFG_TEMPLATE *ptemplate = iec61850_config_template(g_pcfg, "ied");
    if (ptemplate == NULL) {
        return ERR_NOTEXIST;
    }

    DEV_DATA *pdev = match_dev_ied(ptemplate->name, ptemplate->proto);
    if (pdev == NULL) {
        return ERR_NOTEXIST;
    }

    if (pdev->init) {
        return ERR_OK;
    }

    pdev->ptemplate = ptemplate;

    int ret = dev_data_register(pdev);
    if (ret != ERR_OK) {
        return ret;
    }
    pdev->init = 1;

    log_info(GLOG, "ied_data_init ok");

    return ERR_OK;
}

int dev_ied_exit()
{
    IEC61850_CFG_TEMPLATE *ptemplate = iec61850_config_template(g_pcfg, "ied");
    if (ptemplate == NULL) {
        return ERR_NOTEXIST;
    }

    DEV_DATA *pdev = match_dev_ied(ptemplate->name, ptemplate->proto);
    if (pdev != NULL) {
        dev_data_unregister(pdev);
    }
    memset(pdev, 0, sizeof(DEV_DATA));

    return ERR_OK;
}
