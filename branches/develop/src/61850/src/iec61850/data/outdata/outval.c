#include "err_def.h"

#include "outval.h"
#include "outdata.h"
#include "log_conf.h"

int outval_get(OUTVAL *pval)
{
    OUTDATA *pdata = outdata_get(pval->odt);
    if (NULL == pdata) {
        log_error(OUTLOG, "outval_get outdata_get type=%d failed", pval->odt);
        return ERR_NOTEXIST;
    }

    if (pdata->get_val != NULL) {
        return pdata->get_val(pdata, pval);
    }
    else {
        log_error(OUTLOG, "odt=%d outval_get pdata->get_val is NULL", pval->odt);
        return ERR_PNULL;
    }
    return ERR_PNULL;
}

int outval_get_event(OUTVAL_EVENT *pevent)
{
    OUTVAL outval;

    outval.odt = ODT_EMU2000_SHM;
    outval.type = OVT_EVENT | OVM_RECORD;
    outval.struct_val.len = sizeof(OUTVAL_EVENT);
    outval.struct_val.buf = (char *)pevent;

    int ret = outval_get(&outval);
    if (ret < ERR_OK) {
        log_error(OUTLOG, "outval_get_guid outval_get failed ret=%d", ret);
        return ret;
    }

    return ret;
}

int outval_get_event_vals(OUTVAL_EVENT_VALS *pevent)
{
    OUTVAL outval;

    outval.odt = ODT_EMU2000_SHM;
    outval.type = OVT_EVENT_VALS | OVM_RECORD;
    outval.struct_val.len = sizeof(OUTVAL_EVENT_VALS);
    outval.struct_val.buf = (char *)pevent;

    int ret = outval_get(&outval);
    if (ret < ERR_OK) {
        log_error(OUTLOG, "outval_get_guid outval_get failed ret=%d", ret);
        return ret;
    }

    return ret;
}

int outval_get_guid(DEV_DATA *pdev)
{
    if (pdev == NULL) {
        log_error(OUTLOG, "outval_get_guid pdev is NULL");
        return ERR_PTR;
    }

    OUTVAL_PORT_ADDR port_addr = {
        .port = pdev->port,
        .addr = pdev->addr,
        .guid = pdev->guid,
    };
    OUTVAL outval;

    outval.odt = ODT_EMU2000_SHM;
    outval.type = OVT_PORT_ADDR | OVM_DEVDATA;
    outval.struct_val.len = sizeof(OUTVAL_PORT_ADDR);
    outval.struct_val.buf = (char *)&port_addr;

    int ret = outval_get(&outval);
    if (ret != ERR_OK) {
        log_error(OUTLOG, "outval_get_guid outval_get failed ret=%d", ret);
        return ret;
    }

    return ret;
}

int outval_get_commstate(DEV_DATA *pdev)
{
    if (pdev == NULL) {
        log_error(OUTLOG, "outval_get_commstate pdev is NULL");
        return ERR_PTR;
    }

    OUTVAL outval;

    outval.pdev = pdev;
    outval.odt = ODT_EMU2000_SHM;
    outval.type = OVT_DEV_COMMSTATE | OVM_DEVDATA;

    int ret = outval_get(&outval);
    if (ret != ERR_OK) {
        log_error(OUTLOG, "outval_get_commstate outval_get failed ret=%d", ret);
        return ret;
    }
    pdev->commstate = (unsigned char)outval.byval;

    return ret;
}
