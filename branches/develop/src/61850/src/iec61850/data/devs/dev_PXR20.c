#include "err_def.h"
#include "dev_data.h"
#include "iec61850_conf.h"

#include "dev_PXR20.h"
#include "outdata.h"

#include "log_conf.h"

extern IEC61850_CONF *g_pcfg;

/*  */
typedef struct _DEV_PXR20_PRIVATE_DATA {
    OUTDATA_TYPE odt;
} DEV_PXR20_PRIVATE_DATA;

static DEV_PXR20_PRIVATE_DATA gs_private_data = {
    .odt = ODT_EMU2000_SHM,
};

static int outval_updatetime2devaltime(OUTVAL_UPDATE_TIME *t,
                                       DEV_VAL_UPDATE_TIME *dev_t)
{
    if (t == NULL || dev_t == NULL) {
        return -1;
    }

    dev_t->choice = t->choice;
    /* printf("choice=%d\n", t->choice); */
    if (t->choice == 1) {
        memcpy(&dev_t->acsi_time, &t->acsi_time, sizeof(OUT_ACSI_TIMESTAMP));
    }
    else if (t->choice == 2) {
        dev_t->utc_time = t->utc_time;
    }

    return 0;
}

int dev_pxr20_get_val(DEV_DATA *pdev, DEV_VAL *pval)
{
    if (pdev == NULL || pval == NULL) {
        return ERR_PNULL;
    }
    log_debug(DEVLOG, "dev_pxr20_get_val name=%s type=%d", pval->name,
              pval->type);
    DEV_PXR20_PRIVATE_DATA *ppriv = (DEV_PXR20_PRIVATE_DATA *)pdev->priv_data;
    if (NULL == ppriv) {
        return ERR_PNULL;
    }
    OUTVAL outval;

    outval.pdev = pdev;
    outval.name = pval->name;
    outval.odt = ppriv->odt;

    outval.type = OVM_REALTIME;

    int ret = outval_get(&outval);
    if (ret != ERR_OK) {
        log_warn(DEVLOG, "dev_pxr20_get_val outval_get name=%s failed ret=%d",
                 pval->name, ret);
        return ret;
    }

    switch (outval.ntype) {
    case OVT_BOOL: {
        pval->type = DVT_BOOLEAN;
        pval->bval = outval.bval;
        log_debug(DVALOG, "dev_pxr20_get_val name=%s bool=%d", pval->name,
                  pval->bval);
    } break;
    case OVT_CHAR: {
        pval->type = DVT_CHAR;
        pval->cval = outval.cval;
        log_debug(DVALOG, "dev_pxr20_get_val name=%s char=%c", pval->name,
                  pval->cval);
    } break;
    case OVT_BYTE: {
        pval->type = DVT_BYTE;
        pval->byval = outval.byval;
        log_debug(DVALOG, "dev_pxr20_get_val name=%s byte=%d", pval->name,
                  pval->byval);
    } break;
    case OVT_SINT: {
        pval->type = DVT_SHORT;
        pval->sval = outval.sval;
        log_debug(DVALOG, "dev_pxr20_get_val name=%s sint=%d", pval->name,
                  pval->sval);
    } break;
    case OVT_USINT: {
        pval->type = DVT_WORD;
        pval->wval = outval.wval;
        log_debug(DVALOG, "dev_pxr20_get_val name=%s usint=%d", pval->name,
                  pval->wval);
    } break;
    case OVT_INT: {
        pval->type = DVT_INT;
        pval->ival = outval.ival;
        log_debug(DVALOG, "dev_pxr20_get_val name=%s int=%d", pval->name,
                  pval->ival);
    } break;
    case OVT_DBPOS: {
        pval->type = DVT_DBPOS;
        pval->ival = outval.ival;
        log_debug(DVALOG, "dev_pxr20_get_val name=%s int=%d", pval->name,
                  pval->ival);
    } break;
    case OVT_UINT: {
        pval->type = DVT_DWORD;
        pval->doval = outval.doval;
        log_debug(DVALOG, "dev_pxr20_get_val name=%s uint=%u", pval->name,
                  pval->doval);
    } break;
    case OVT_I64: {
        pval->type = DVT_INT;
        pval->ival = outval.i64val;
        log_debug(DVALOG, "dev_pxr20_get_val name=%s i64=%lld", pval->name,
                  (long long)pval->ival);
    } break;
    case OVT_U64: {
        pval->type = DVT_DWORD;
        pval->doval = outval.u64val;
        log_debug(DVALOG, "dev_pxr20_get_val name=%s u64=%llu", pval->name,
                  (unsigned long long)pval->doval);
    } break;
    case OVT_FLOAT: {
        pval->type = DVT_FLOAT;
        pval->fval = outval.fval;
        log_debug(DVALOG, "dev_pxr20_get_val name=%s float=%f", pval->name,
                  pval->fval);
    } break;
    case OVT_DOUBLE: {
        pval->type = DVT_DOUBLE;
        pval->dval = outval.dval;
        log_debug(DVALOG, "dev_pxr20_get_val name=%s double=%f", pval->name,
                  pval->dval);
    } break;

    case OVT_STRING128: {
        pval->type = DVT_STRING;
        pval->str.val = strdup(outval.struct_val.buf);
        log_debug(DVALOG, "dev_pxr20_get_val name=%s str=%s", pval->name,
                  pval->str.val);
    } break;
    case OVT_TIME: {
        pval->type = DVT_UTCTIME;
        pval->utc_time = outval.u64val;
        log_debug(DVALOG, "dev_pxr20_get_val name=%s utc_time=%llu", pval->name,
                  (unsigned long long)pval->doval);
    } break;

    default:
        log_warn(DEVLOG, "dev_pxr20_get_val unsupported type=%x", outval.ntype);
        pval->type = DVT_INVALID;
        return ERR_TYPE;
        break;
    }

    outval_updatetime2devaltime(&outval.t, &pval->t);
    /* printf("%s update_time:%llu\n", pval->name, */
    /*        pval->t.choice == 1 ? pval->t.acsi_time.SecondSinceEpoch : */
    /*                              pval->t.utc_time); */
    if (pval->t.choice == 1) {
        /* log_debug(DEVLOG, "%s update_time choice=1 acsi_time=%u", pval->name, */
        /*           pval->t.acsi_time.SecondSinceEpoch); */
        /* printf("%s acsi_time: %u.%u\n", pval->name, */
        /*        pval->t.acsi_time.SecondSinceEpoch, */
        /*        pval->t.acsi_time.FractionOfSecond); */
    }
    else if (pval->t.choice == 2) {
        /* log_debug(DEVLOG, "%s update_time choice=2 utc_time=%llu", pval->name, */
        /*           pval->t.utc_time); */
        /* printf("%s utc_time: %llu\n", pval->name, */
        /*        (unsigned long long)pval->t.utc_time); */
    }

    return ERR_OK;
}

int dev_pxr20_get_list_val(DEV_DATA *pdev, DEV_VAL_LIST *pval)
{
    if (pdev == NULL || pval == NULL) {
        return ERR_PNULL;
    }

    switch (pval->type) {
    case DVLT_TEMPLATE: {
        IEC61850_CFG_TEMPLATE *ptemplate = pdev->ptemplate;
        if (ptemplate == NULL) {
            return ERR_PNULL;
        }

        pval->num = ptemplate->dataset_list.num;
        pval->vals = (DEV_VAL *)malloc(sizeof(DEV_VAL) * pval->num);
        if (pval->vals == NULL) {
            return ERR_MEM;
        }

        for (int i = 0; i < ptemplate->dataset_list.num; i++) {
            IEC61850_CFG_TEMPLATE_DATASET *pdataset =
                &ptemplate->dataset_list.dataset[i];
            DEV_VAL *val = pval->vals + i;
            memset(val, 0, sizeof(DEV_VAL));
            val->name = strdup(pdataset->name);

            char refname[256];
            val->refname = NULL;
            if (pdataset->ref_name) {
                snprintf(refname, sizeof(refname), "%s/%s", pdev->devname,
                         pdataset->ref_name);
                val->refname = strdup(refname);
            }

            val->ref_time = NULL;
            if (pdataset->ref_time) {
                snprintf(refname, sizeof(refname), "%s/%s", pdev->devname,
                         pdataset->ref_time);
                val->ref_time = strdup(refname);
            }
            if (pdataset->ref_q) {
                snprintf(refname, sizeof(refname), "%s/%s", pdev->devname,
                         pdataset->ref_q);
                val->ref_q = strdup(refname);
            }
        }
    }; break;
    default:
        break;
    }

    if (pval->num <= 0 || pval->vals == NULL) {
        return ERR_PNULL;
    }
    for (int i = 0; i < pval->num; i++) {
        int ret = dev_pxr20_get_val(pdev, &pval->vals[i]);
        if (ret != ERR_OK) {
            log_warn(DEVLOG,
                     "dev_pxr20_get_list_val get_val name=%s failed ret=%d",
                     pval->vals[i].name, ret);

            pval->vals[i].type = DVT_INVALID;
        }

        int commstate = outval_get_commstate(pdev);
        if (commstate != 0) {
            pval->vals[i].type = DVT_COMMSTATE_ERROR;

            log_warn(DEVLOG,
                     "dev_pxr20_get_list_val name=%s communication error, "
                     "commstate=%d",
                     pval->vals[i].name, commstate);
            /* printf("dev_pxr20_get_list_val name=%s communication error\n", */
            /*        pval->vals[i].name); */
        }
    }

    return ERR_OK;
}

int dev_pxr20_get_record(DEV_DATA *pdev, DEV_VAL *pval)
{
    if (pdev == NULL || pval == NULL) {
        return ERR_PNULL;
    }
    log_debug(DEVLOG, "dev_pxr20_get_val name=%s type=%d", pval->name,
              pval->type);
    DEV_PXR20_PRIVATE_DATA *ppriv = (DEV_PXR20_PRIVATE_DATA *)pdev->priv_data;
    if (NULL == ppriv) {
        return ERR_PNULL;
    }
    OUTVAL outval;

    outval.pdev = pdev;
    outval.name = pval->name;
    outval.odt = ppriv->odt;

    outval.type = OVM_RECORD;

    int ret = outval_get(&outval);
    if (ret != ERR_OK) {
        log_warn(DEVLOG, "dev_pxr20_get_val outval_get name=%s failed ret=%d",
                 pval->name, ret);
        return ret;
    }

    switch (outval.ntype) {
    case OVT_BOOL: {
        pval->type = DVT_BOOLEAN;
        pval->bval = outval.bval;
        log_debug(DVALOG, "dev_pxr20_get_val name=%s bool=%d", pval->name,
                  pval->bval);
    } break;
    case OVT_CHAR: {
        pval->type = DVT_CHAR;
        pval->cval = outval.cval;
        log_debug(DVALOG, "dev_pxr20_get_val name=%s char=%c", pval->name,
                  pval->cval);
    } break;
    case OVT_BYTE: {
        pval->type = DVT_BYTE;
        pval->byval = outval.byval;
        log_debug(DVALOG, "dev_pxr20_get_val name=%s byte=%d", pval->name,
                  pval->byval);
    } break;
    case OVT_SINT: {
        pval->type = DVT_SHORT;
        pval->sval = outval.sval;
        log_debug(DVALOG, "dev_pxr20_get_val name=%s sint=%d", pval->name,
                  pval->sval);
    } break;
    case OVT_USINT: {
        pval->type = DVT_WORD;
        pval->wval = outval.wval;
        log_debug(DVALOG, "dev_pxr20_get_val name=%s usint=%d", pval->name,
                  pval->wval);
    } break;
    case OVT_INT: {
        pval->type = DVT_INT;
        pval->ival = outval.ival;
        log_debug(DVALOG, "dev_pxr20_get_val name=%s int=%d", pval->name,
                  pval->ival);
    } break;
    case OVT_UINT: {
        pval->type = DVT_DWORD;
        pval->doval = outval.doval;
        log_debug(DVALOG, "dev_pxr20_get_val name=%s uint=%u", pval->name,
                  pval->doval);
    } break;
    case OVT_I64: {
        pval->type = DVT_INT;
        pval->ival = outval.i64val;
        log_debug(DVALOG, "dev_pxr20_get_val name=%s i64=%lld", pval->name,
                  (long long)pval->ival);
    } break;
    case OVT_U64: {
        pval->type = DVT_DWORD;
        pval->doval = outval.u64val;
        log_debug(DVALOG, "dev_pxr20_get_val name=%s u64=%llu", pval->name,
                  (unsigned long long)pval->doval);
    } break;
    case OVT_FLOAT: {
        pval->type = DVT_FLOAT;
        pval->fval = outval.fval;
        log_debug(DVALOG, "dev_pxr20_get_val name=%s float=%f", pval->name,
                  pval->fval);
    } break;
    case OVT_DOUBLE: {
        pval->type = DVT_DOUBLE;
        pval->dval = outval.dval;
        log_debug(DVALOG, "dev_pxr20_get_val name=%s double=%f", pval->name,
                  pval->dval);
    } break;

    default:
        log_warn(DEVLOG, "dev_pxr20_get_val unsupported type=%x", outval.ntype);
        return ERR_TYPE;
        pval->type = DVT_INVALID;
        break;
    }

    return ERR_OK;
}

DEV_DATA gs_dev_data_pxr20[] = {
    {
        .name = "pxr20",
        .proto = TEMPLATE_PROTO_EMU2000_SHM,
        .init = 0,
        .ptemplate = NULL,
        .set_val = NULL,
        .get_val = dev_pxr20_get_val,
        .get_list_val = dev_pxr20_get_list_val,
        .get_record = dev_pxr20_get_record,
        .priv_data = &gs_private_data,
    },
    {
        .name = "pxr25",
        .proto = TEMPLATE_PROTO_EMU2000_SHM,
        .init = 0,
        .ptemplate = NULL,
        .set_val = NULL,
        .get_val = dev_pxr20_get_val,
        .get_list_val = dev_pxr20_get_list_val,
        .get_record = dev_pxr20_get_record,
        .priv_data = &gs_private_data,
    },
};

/**
 *  \brief match 装置数据
 *  \param name protocol
 *  \return NULL-失败 其他-成功
 */
static DEV_DATA *match_dev_pxr20(char *name, char *protocol)
{
    if (name == NULL) {
        return NULL;
    }

    for (int i = 0;
         i < (int)(sizeof(gs_dev_data_pxr20) / sizeof(gs_dev_data_pxr20[0]));
         i++) {
        DEV_DATA *pdev = &gs_dev_data_pxr20[i];
        if (strcmp(pdev->name, name) == 0 &&
            strcmp(pdev->proto, protocol) == 0) {
            return pdev;
        }
    }

    return NULL;
}

/**
 *  \brief 注册两种装置
 *  \param name
 *  \return
 */
int register_dev_pxr20(char *name)
{
    if (g_pcfg == NULL) {
        return ERR_PNULL;
    }

    IEC61850_CFG_TEMPLATE *ptemplate = iec61850_config_template(g_pcfg, name);
    if (ptemplate == NULL) {
        return ERR_NOTEXIST;
    }

    DEV_DATA *pdev = match_dev_pxr20(ptemplate->name, ptemplate->proto);
    if (pdev == NULL) {
        return ERR_NOTEXIST;
    }

    if (pdev->init) {
        return ERR_OK;
    }

    pdev->ptemplate = ptemplate;
    pdev->init = 1;
    int ret = dev_data_register(pdev);
    if (ret != ERR_OK) {
        return ret;
    }

    return ERR_OK;
}

int dev_pxr20_init()
{
    int i;
    for (i = 0;
         i < (int)(sizeof(gs_dev_data_pxr20) / sizeof(gs_dev_data_pxr20[0]));
         i++) {
        DEV_DATA *pdev = &gs_dev_data_pxr20[i];
        int ret = register_dev_pxr20(pdev->name);
        if (ret == ERR_OK) {
            log_info(DEVLOG, "dev_pxr20_init register name=%s proto=%s ok",
                     pdev->name, pdev->proto);
        }
        else {
            log_error(DEVLOG,
                      "dev_pxr20_init register name=%s proto=%s failed "
                      "ret=%d(%s)",
                      pdev->name, pdev->proto, ret, err_str(ret));
        }
    }

    return ERR_OK;
}
int dev_pxr20_exit()
{
    return ERR_OK;
}
