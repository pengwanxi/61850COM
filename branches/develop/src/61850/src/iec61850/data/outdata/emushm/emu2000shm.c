#include <time.h>
#include <sys/time.h>

#include "err_def.h"
#include "log_conf/log_conf.h"

#include "rdbDef.h"
#include "sharem.h"
#include "semph.h"
#include "iec61850_macro.h"
#include "emu2000shm.h"
#include "dev_list.h"
#include "emu2000ini_busxx.h"
#include "ini.h"

static int gs_timezone = 0;
static long long int gs_diffutc_ms = 0;
#define NTPCLIENT_INI_MATCH(s, n)                                              \
    strcmp(section, s) == 0 && strcmp(name, n) == 0
/**
 *  \brief busline 的读取处理
 *  \param
 *  \return 1成功
 */
static int ntpclient_info_handle(void *user, const char *section,
                                 const char *name, const char *value)
{
    int *p = (int *)user;
    if (NTPCLIENT_INI_MATCH("NtpClient", "timezone")) {
        *p = atoi(value);
    }
    else {
        return 0; /* unknown section/name, error */
    }

    return 1;
}

extern EMU2000_DEVTYPE_DATA g_emu2000_devtype_data;

static SHAREM_DATA g_emu2000_shm_data = {
    .sharem_id = EMU2000_SHMDBKEY,
    .size = sizeof(SHM_SPACE),
    .psharem = NULL,
};

static SEMPH_DATA g_emu2000_semph_data = {
    .semph_id = EMU2000_SHMDBKEY + 1,
};

/*  */
typedef struct _EMU2000SHM_PRIVATE_DATA {
    SHAREM_DATA *pshm_data;
    SEMPH_DATA *psemph_data;
} EMU2000SHM_PRIVATE_DATA;

EMU2000SHM_PRIVATE_DATA g_emu2000shm_priv_data = {
    .pshm_data = &g_emu2000_shm_data,
    .psemph_data = &g_emu2000_semph_data,
};

static IEC61850_CFG_TEMPLATE_DATASET *emu2000shm_match_dataset(OUTVAL *pval)
{
    DEV_DATA *pdev = pval->pdev;
    if (pdev == NULL) {
        OUT_LOG_WARN_BUF("emu2000shm_match_dataset", "pdev is NULL");
        return NULL;
    }

    IEC61850_CFG_TEMPLATE *ptemplate = pdev->ptemplate;
    if (ptemplate == NULL) {
        OUT_LOG_WARN_BUF("emu2000shm_match_dataset", "ptemplate is NULL");
        return NULL;
    }

    int i;
    for (i = 0; i < ptemplate->dataset_list.num; i++) {
        IEC61850_CFG_TEMPLATE_DATASET *pdataset =
            &ptemplate->dataset_list.dataset[i];
        if (0 == strcmp(pval->name, pdataset->name)) {
            return pdataset;
        }
    }

    return NULL;
};

static int set_update_asci_time(OUTVAL_UPDATE_TIME *pval, ACSI_TIMESTAMP *t)
{
    pval->choice = 0;
    if (t->SecondSinceEpoch) {
        pval->choice = 1;
        pval->acsi_time.SecondSinceEpoch = t->SecondSinceEpoch;
        pval->acsi_time.FractionOfSecond = t->FractionOfSecond;
        pval->acsi_time.TimeQuality = t->TimeQuality;
    }

    OUT_LOG_DEBUG_BUF("set_update_asci_time",
                       "choice=%d acsi_time=%u.%u quality=%u", pval->choice,
                       pval->acsi_time.SecondSinceEpoch,
                       pval->acsi_time.FractionOfSecond,
                       pval->acsi_time.TimeQuality);
    return 0;
}

static int emu2000shm_get_realtime_data(OUTDATA *pdata, OUTVAL *pval)
{
    IEC61850_CFG_TEMPLATE_DATASET *pdataset = emu2000shm_match_dataset(pval);
    if (pdataset == NULL) {
        OUT_LOG_WARN_BUF("emu2000shm_get_realtime_data",
                         "emu2000shm_match_dataset %s failed", pval->name);
        return ERR_O_RANGE;
    }
    SHM_SPACE *psharem = (SHM_SPACE *)g_emu2000_shm_data.psharem;
    if (psharem == NULL) {
        OUT_LOG_WARN_BUF("emu2000shm_get_realtime_data", "psharem is NULL");
        return ERR_PNULL;
    }

    int dev_no = atoi(pval->pdev->guid);
    if (dev_no <= 0 || dev_no > psharem->sysInfo.wStnSum) {
        OUT_LOG_WARN_BUF("emu2000shm_get_realtime_data", "invalid dev_no=%d",
                         dev_no);
        return ERR_O_RANGE;
    }
    OUT_LOG_DEBUG_BUF("emu2000shm_get_realtime_data",
                      "stnsum=%d type=%s idx=%s", psharem->sysInfo.wStnSum,
                      pdataset->data_type, pdataset->data_idx);

    STNPARAM *pstn = &psharem->RTDBase.StnUnit[dev_no - 1];
    if (pstn == NULL) {
        OUT_LOG_WARN_BUF("emu2000shm_get_realtime_data",
                         "pstn is NULL dev_no=%d", dev_no);
        return ERR_PNULL;
    }

    if (0 == strcmp(pdataset->data_type, EMU2000_DATA_TYPE_YC)) {
        int data_idx = atoi(pdataset->data_idx);
        if (data_idx <= 0 || data_idx > pstn->wAnalogSum) {
            OUT_LOG_WARN_BUF("emu2000shm_get_realtime_data",
                             "invalid YC data_idx=%d sum=%d", data_idx,
                             pstn->wAnalogSum);
            return ERR_O_RANGE;
        }

        int idx0 = data_idx - 1;

        pval->type = OVT_FLOAT | OVM_REALTIME;

        ANALOGITEM *pa =
            &psharem->RTDBase.AnalogTable[pstn->dwAnalogPos + idx0];
        pval->fval = pa->fRealVal;
        /* printf("YC point=%d %f Seconds=%d\n",pstn->dwAnalogPos + idx0,pval->fval, pa->ACSITime.SecondSinceEpoch); */
        set_update_asci_time(&pval->t, &pa->ACSITime);
        /* printf("update_time choice=%d acsi_time=%u\n", pval->t.choice, */
        /* pval->t.acsi_time.SecondSinceEpoch); */
        OUT_LOG_DEBUG_BUF("emu2000shm_get_realtime_data", "fval=%f",
                          pval->fval);
        return ERR_OK;
    }
    else if (0 == strcmp(pdataset->data_type, EMU2000_DATA_TYPE_YXPOS)) {
        int data_idx = atoi(pdataset->data_idx);
        if (data_idx <= 0 || data_idx > pstn->wDigitalSum) {
            OUT_LOG_WARN_BUF("emu2000shm_get_realtime_data",
                             "invalid YX data_idx=%d sum=%d", data_idx,
                             pstn->wDigitalSum);
            return ERR_O_RANGE;
        }
        int idx0 = data_idx - 1;

        pval->type = OVT_DBPOS | OVM_REALTIME;
        DIGITALITEM *pd =
            &psharem->RTDBase.DigitalTable[pstn->dwDigitalPos + idx0];

        if ((pd->wStatus & DISTATUS_VALID) == 0) {
            pval->ival = 0;
            OUT_LOG_DEBUG_BUF("emu2000shm_get_realtime_data", "status invalid");
            return ERR_OK;
        }

        unsigned char val = pd->wStatus & DISTATUS_VALUE;
        switch (val) {
        case 1: {
            pval->ival = 2;
        } break;
        default:
            pval->ival = 1;
            break;
        }
        set_update_asci_time(&pval->t, &pd->ACSITime);

        OUT_LOG_DEBUG_BUF("emu2000shm_get_realtime_data",
                          "pnt=%d type=%d attr=%d ctrl=%d evtcode=%d "
                          "reseved=%d status=%d",
                          pd->wPntID, pd->byType, pd->byAttr, pd->wPntCtrl,
                          pd->wEvtCode, pd->wReserve, pd->wStatus);

        OUT_LOG_DEBUG_BUF("emu2000shm_get_realtime_data", "ival=%d",
                          pval->ival);
        return ERR_OK;
    }
    else if (0 == strcmp(pdataset->data_type, EMU2000_DATA_TYPE_YX)) {
        int data_idx = atoi(pdataset->data_idx);
        if (data_idx <= 0 || data_idx > pstn->wDigitalSum) {
            OUT_LOG_WARN_BUF("emu2000shm_get_realtime_data",
                             "invalid YX data_idx=%d sum=%d", data_idx,
                             pstn->wDigitalSum);
            return ERR_O_RANGE;
        }

        int idx0 = data_idx - 1;

        pval->type = OVT_BOOL | OVM_REALTIME;
        DIGITALITEM *pd =
            &psharem->RTDBase.DigitalTable[pstn->dwDigitalPos + idx0];
        if ((pd->wStatus & DISTATUS_VALID) == 0) {
            pval->bval = 0;
            OUT_LOG_DEBUG_BUF("emu2000shm_get_realtime_data", "status invalid");
            return ERR_OK;
        }
        pval->bval = pd->wStatus & DISTATUS_VALUE;
        set_update_asci_time(&pval->t, &pd->ACSITime);

        OUT_LOG_DEBUG_BUF("emu2000shm_get_realtime_data",
                          "pnt=%d type=%d attr=%d ctrl=%d evtcode=%d "
                          "reseved=%d status=%d",
                          pd->wPntID, pd->byType, pd->byAttr, pd->wPntCtrl,
                          pd->wEvtCode, pd->wReserve, pd->wStatus);

        OUT_LOG_DEBUG_BUF("emu2000shm_get_realtime_data", "bval=%d",
                          pval->bval);
        return ERR_OK;
    }
    else if (0 == strcmp(pdataset->data_type, EMU2000_DATA_TYPE_YM)) {
        int data_idx = atoi(pdataset->data_idx);
        if (data_idx <= 0 || data_idx > pstn->wPulseSum) {
            OUT_LOG_WARN_BUF("emu2000shm_get_realtime_data",
                             "invalid YM data_idx=%d sum=%d", data_idx,
                             pstn->wPulseSum);
            return ERR_O_RANGE;
        }
        int idx0 = data_idx - 1;

        pval->type = OVT_U64 | OVM_REALTIME;

        PULSEITEM *pp = &psharem->RTDBase.PulseTable[pstn->dwPulsePos + idx0];
        pval->u64val = pp->dwRawVal;
        set_update_asci_time(&pval->t, &pp->ACSITime);

        OUT_LOG_DEBUG_BUF("emu2000shm_get_realtime_data", "val=%llu",
                          pval->u64val);
        return ERR_OK;
    }
    else if (0 == strcmp(pdataset->data_type, EMU2000_DATA_TYPE_YM_TIME)) {
        int data_idx = atoi(pdataset->data_idx);
        if (data_idx <= 0 || data_idx > pstn->wPulseSum) {
            OUT_LOG_WARN_BUF("emu2000shm_get_realtime_data",
                             "invalid YM data_idx=%d sum=%d", data_idx,
                             pstn->wPulseSum);
            return ERR_O_RANGE;
        }
        int idx0 = data_idx - 1;

        pval->type = OVT_TIME | OVM_REALTIME;

        PULSEITEM *pp = &psharem->RTDBase.PulseTable[pstn->dwPulsePos + idx0];
        pval->u64val = pp->dwRawVal * 1000;
        set_update_asci_time(&pval->t, &pp->ACSITime);

        OUT_LOG_DEBUG_BUF("emu2000shm_get_realtime_data", "val=%llu",
                          pval->u64val);
        return ERR_OK;
    }
    else if (0 == strcmp(pdataset->data_type, EMU2000_DATA_TYPE_YM_ALM)) {
        int data_idx = atoi(pdataset->data_idx);
        if (data_idx <= 0 || data_idx > pstn->wPulseSum) {
            OUT_LOG_WARN_BUF("emu2000shm_get_realtime_data",
                             "invalid YM data_idx=%d sum=%d", data_idx,
                             pstn->wPulseSum);
            return ERR_O_RANGE;
        }
        int idx0 = data_idx - 1;

        pval->type = OVT_BOOL | OVM_REALTIME;

        PULSEITEM *pp = &psharem->RTDBase.PulseTable[pstn->dwPulsePos + idx0];
        if ( pp->dwRawVal == 3 || pp->dwRawVal == 4) {
            pval->bval = 1;
        }
        else {
            pval->bval = 0;
        }
        set_update_asci_time(&pval->t, &pp->ACSITime);

        OUT_LOG_DEBUG_BUF("emu2000shm_get_realtime_data ym_alm", "val=%llu yx=%d",
                          pval->u64val, pval->ival);
        return ERR_OK;
    }
    else if (0 == strcmp(pdataset->data_type, EMU2000_DATA_TYPE_LDNAME)) {
        DEV_DATA *pdev = pval->pdev;
        pval->type = OVT_STRING128 | OVM_REALTIME;
        pval->struct_val.len = 128;
        pval->struct_val.buf = pdev->ptemplate->lln0.NamPlt.vendor;
        OUT_LOG_DEBUG_BUF("emu2000shm_get_realtime_data", "ldname=%s",
                          pval->struct_val.buf);
    }
    else if (0 == strcmp(pdataset->data_type, EMU2000_DATA_TYPE_SWREV)) {
        DEV_DATA *pdev = pval->pdev;
        pval->type = OVT_STRING128 | OVM_REALTIME;
        pval->struct_val.len = 128;
        pval->struct_val.buf = pdev->ptemplate->lln0.NamPlt.swRev;
        OUT_LOG_DEBUG_BUF("emu2000shm_get_realtime_data", "swrev=%s",
                          pval->struct_val.buf);
    }
    else if (0 == strcmp(pdataset->data_type, EMU2000_DATA_TYPE_PLTD)) {
        DEV_DATA *pdev = pval->pdev;
        pval->type = OVT_STRING128 | OVM_REALTIME;
        pval->struct_val.len = 128;
        pval->struct_val.buf = pdev->ptemplate->lln0.NamPlt.d;
        OUT_LOG_DEBUG_BUF("emu2000shm_get_realtime_data", "pltd=%s",
                          pval->struct_val.buf);
    }
    else if (0 == strcmp(pdataset->data_type, EMU2000_DATA_TYPE_CONFREV)) {
        DEV_DATA *pdev = pval->pdev;
        pval->type = OVT_STRING128 | OVM_REALTIME;
        pval->struct_val.len = 128;
        pval->struct_val.buf = pdev->ptemplate->lln0.NamPlt.configRev;
        OUT_LOG_DEBUG_BUF("emu2000shm_get_realtime_data", "confrev=%s",
                          pval->struct_val.buf);
    }
    else if (0 == strcmp(pdataset->data_type, EMU2000_DATA_TYPE_LDNS)) {
        int data_idx = atoi(pdataset->data_idx);
        if (data_idx <= 0 || data_idx > pstn->wPulseSum) {
            OUT_LOG_WARN_BUF("emu2000shm_get_realtime_data",
                             "invalid YM data_idx=%d sum=%d", data_idx,
                             pstn->wPulseSum);
            return ERR_O_RANGE;
        }
        int idx0 = data_idx - 1;
        pval->type = OVT_U64 | OVM_REALTIME;
        PULSEITEM *pp = &psharem->RTDBase.PulseTable[pstn->dwPulsePos + idx0];
        pval->u64val = pp->dwRawVal;

        DEV_DATA *pdev = pval->pdev;
        pval->type = OVT_STRING128 | OVM_REALTIME;
        pval->struct_val.len = 128;
        if (pval->u64val == 1) {
            pval->struct_val.buf = "PXR20";
        }
        else if (pval->u64val == 2) {
            pval->struct_val.buf = "PXR25";
        }
        else {
            pval->struct_val.buf = "UNKNOWN";
        }

        OUT_LOG_DEBUG_BUF("emu2000shm_get_realtime_data", "ldns=%s(%d)",
                          pval->struct_val.buf, pval->u64val);
    }

    else {
        OUT_LOG_WARN_BUF("emu2000shm_get_realtime_data",
                         "unsupported data_type=%s", pdataset->data_type);
        return ERR_TYPE;
    }

    return ERR_OK;
}

static int emu2000shm_get_devtype_data(OUTDATA *pdata, OUTVAL *pval)
{
    SHM_SPACE *psharem = (SHM_SPACE *)g_emu2000_shm_data.psharem;
    if (psharem == NULL) {
        OUT_LOG_WARN_BUF("emu2000shm_get_realtime_data", "psharem is NULL");
        return ERR_PNULL;
    }

    int i;
    for (i = 0; i < psharem->sysInfo.wStnSum; i++) {
        STNPARAM *pstn = &psharem->RTDBase.StnUnit[i];
        if (pstn == NULL) {
            OUT_LOG_WARN_BUF("emu2000shm_get_realtime_data",
                             "pstn is NULL dev_no=%d", i);
            return ERR_PNULL;
        }
        /* printf("stn=%p %p\n", (char *)pstn - (char *)psharem, */
        /*        (char *)pstn->pDigitalTab - (char *)psharem); */

        int idx0 = 0;

        pval->type = OVT_U64 | OVM_REALTIME;

        PULSEITEM *pp = &psharem->RTDBase.PulseTable[pstn->dwPulsePos + idx0];
        pval->u64val = pp->dwRawVal;

        if (pval->u64val >= EMU2000_DEVTYPE_NULL &&
            pval->u64val <= EMU2000_DEVTYPE_SIZE) {
            /* 当装置类型不同时，认为是自动识别时装置改变了，因为61850重启服务太过麻烦，系统会重启程序，程序退出让系统重启 */
            if ((g_emu2000_devtype_data.devtype[i] != pval->u64val) &&
                (g_emu2000_devtype_data.devtype[i] != 0)) {
                exit(-1);
            }
            g_emu2000_devtype_data.devtype[i] = pval->u64val;
            g_emu2000_devtype_data.mask |= (1U << i);
        }
        else {
            OUT_LOG_WARN_BUF("emu2000shm_get_realtime_data",
                             "invalid devtype=%llu", pval->u64val);
        }
        OUT_LOG_DEBUG_BUF("emu2000shm_get_devtype_data", "val=%llu",
                          pval->u64val);
    }

    if (g_emu2000_devtype_data.num == 0) {
        g_emu2000_devtype_data.num = psharem->sysInfo.wStnSum;
    }

    return g_emu2000_devtype_data.num;
}

static int emu2000shm_get_guid(OUTDATA *pdata, OUTVAL *pval)
{
    SHM_SPACE *psharem = (SHM_SPACE *)g_emu2000_shm_data.psharem;
    if (psharem == NULL) {
        OUT_LOG_WARN_BUF("emu2000shm_get_guid", "psharem is NULL");
        return ERR_PNULL;
    }

    OUTVAL_PORT_ADDR *pport_addr = (OUTVAL_PORT_ADDR *)pval->struct_val.buf;
    if (pport_addr == NULL ||
        pval->struct_val.len != sizeof(OUTVAL_PORT_ADDR)) {
        OUT_LOG_WARN_BUF("emu2000shm_get_guid", "invalid struct_val");
        return ERR_O_RANGE;
    }

    int i;
    for (i = 0; i < psharem->sysInfo.wStnSum; i++) {
        RTDBASE *pdb = &psharem->RTDBase;
        BYTE port = atoi(pport_addr->port);
        WORD addr = atoi(pport_addr->addr);
        OUT_LOG_DEBUG_BUF("emu2000shm_get_guid",
                          "port=%d addr=%d stn=%d busno=%d devaddr=%d", port,
                          addr, i + 1, pdb->StnBusAddr[i].byBusNo,
                          pdb->StnBusAddr[i].wDevAddr);
        if (port == pdb->StnBusAddr[i].byBusNo &&
            addr <= pdb->StnBusAddr[i].wDevAddr) {
            char guid[128];
            snprintf(guid, sizeof(guid), "%d", i + 1);
            if (0 == strcmp(pport_addr->guid, guid)) {
                return ERR_OK;
            }
            /* pport_addr->guid points to DEV_DATA.guid (fixed-size buffer) */
            snprintf(pport_addr->guid, 128, "%d", i + 1);
            return ERR_OK;
        }
    }

    return ERR_O_RANGE;
};

static int emu2000shm_get_commstate(OUTDATA *pdata, OUTVAL *pval)
{
    SHM_SPACE *psharem = (SHM_SPACE *)g_emu2000_shm_data.psharem;
    if (psharem == NULL) {
        OUT_LOG_WARN_BUF("emu2000shm_get_guid", "psharem is NULL");
        return ERR_PNULL;
    }

    int dev_idx = atoi(pval->pdev->guid);
    if (dev_idx <= 0 || dev_idx > psharem->sysInfo.wStnSum) {
        OUT_LOG_WARN_BUF("emu2000shm_get_commstate", "invalid dev_idx=%d",
                         dev_idx);
        return ERR_O_RANGE;
    }

    unsigned char commstate =
        psharem->RTDBase.StnComStatus.byDevComStatus[dev_idx - 1];

    pval->type = OVT_BYTE | OVM_DEVDATA;
    pval->byval = commstate;
    /* printf("pval->byval=%d\n", pval->byval); */

    return pval->byval;
};

static char *emu2000shm_get_name(DEV_DATA *pdev, char *type, unsigned short pnt)
{
    if (pdev == NULL || type == NULL) {
        OUT_LOG_WARN_BUF("emu2000shm_get_name", "invalid param");
        return NULL;
    }

    IEC61850_CFG_TEMPLATE *ptemplate = pdev->ptemplate;
    if (ptemplate == NULL) {
        OUT_LOG_WARN_BUF("emu2000shm_get_name", "ptemplate is NULL");
        return NULL;
    }

    int i;
    for (i = 0; i < ptemplate->event_list.num; i++) {
        IEC61850_CFG_TEMPLATE_EVENT *pevent = &ptemplate->event_list.event[i];
        if (atoi(pevent->yx_idx) == pnt) {
            return pevent->name;
        }
    }
    /* for (i = 0; i < ptemplate->dataset_list.num; i++) { */
    /*     IEC61850_CFG_TEMPLATE_DATASET *pdataset = */
    /*         &ptemplate->dataset_list.dataset[i]; */
    /*     if (0 == strcmp(type, pdataset->data_type) && */
    /*         atoi(pdataset->data_idx) == pnt) { */
    /*         return pdataset->name; */
    /*     } */
    /* } */

    return NULL;
};

static int emu2000shm_get_soe(OUTDATA *pdata, OUTVAL *pval)
{
    SHM_SPACE *psharem = (SHM_SPACE *)g_emu2000_shm_data.psharem;
    if (psharem == NULL) {
        OUT_LOG_WARN_BUF("emu2000shm_get_soe", "psharem is NULL");
        return ERR_PNULL;
    }

    /* 定初始位置 */
    static int soe_pos = -1;
    if (soe_pos == -1) {
        soe_pos = psharem->sysInfo.nSOEWritePos;
        return ERR_OK;
    }

    if (soe_pos == psharem->sysInfo.nSOEWritePos) {
        return ERR_OK;
    }

    OUT_LOG_DEBUG_BUF("emu2000shm_get_soe", "soe_pos=%d write_pos=%d", soe_pos,
                      psharem->sysInfo.nSOEWritePos);

    SOEITEM *psoe = &psharem->soeArray[soe_pos];
    soe_pos += 1;
    soe_pos = soe_pos % SOE_QUEUE_SUM;

    OUTVAL_EVENT *pevent = (OUTVAL_EVENT *)pval->struct_val.buf;
    if (pevent == NULL || pval->struct_val.len != sizeof(OUTVAL_EVENT)) {
        OUT_LOG_WARN_BUF("emu2000shm_get_soe", "invalid struct_val");
        return ERR_O_RANGE;
    }

    char guid[32];
    char addr[32];
    snprintf(guid, sizeof(guid), "%d", psoe->wStnID + 1);

    pevent->pdev = dev_list_data_byguid(guid);
    if (pevent->pdev == NULL) {
        OUT_LOG_WARN_BUF("emu2000shm_get_soe",
                         "dev_list_find_by_guid failed guid=%s", guid);
        return ERR_O_RANGE;
    }

    pevent->event_name = emu2000shm_get_name(pevent->pdev, EMU2000_DATA_TYPE_YX,
                                             psoe->wPntNum + 1);
    if (pevent->event_name == NULL) {
        OUT_LOG_WARN_BUF("emu2000shm_get_soe",
                         "emu2000shm_get_name failed guid=%s type=%s pnt=%d",
                         guid, EMU2000_DATA_TYPE_YX, psoe->wPntNum);
        return ERR_O_RANGE;
    }
    pevent->attr = psoe->wAttrib;
    pevent->bval = psoe->byState;

    OUT_LOG_DEBUG_BUF("emu2000shm_get_soe",
                      "Get SOE event guid=%s event=%s state=%d time=%ld %d",
                      guid, pevent->event_name, pevent->bval, psoe->lTime,
                      psoe->wMiSecond);

    pevent->time.utc_time = (uint64_t)psoe->lTime * 1000 + psoe->wMiSecond;
    /* pevent->time.utc_time += (gs_diffutc_ms); */


    OUT_LOG_INFO_BUF("emu2000shm_get_soe",
                     "Get SOE event %p guid=%s event=%s state=%d time=%lu %lu",
                     pevent, guid, pevent->event_name, pevent->bval,
                     pevent->time.utc_time / 1000,
                     pevent->time.utc_time % 1000);

    return 1;
};
static IEC61850_CFG_TEMPLATE_EVENT_DATA *
emu2000shm_get_event_data(DEV_DATA *pdev, unsigned short pnt)
{
    if (pdev == NULL) {
        OUT_LOG_WARN_BUF("emu2000shm_match_event", "pdev is NULL");
        return NULL;
    }

    IEC61850_CFG_TEMPLATE *ptemplate = pdev->ptemplate;
    if (ptemplate == NULL) {
        OUT_LOG_WARN_BUF("emu2000shm_match_event", "ptemplate is NULL");
        return NULL;
    }

    int i;
    for (i = 0; i < ptemplate->event_data_list.num; i++) {
        IEC61850_CFG_TEMPLATE_EVENT_DATA *pevent_data =
            &ptemplate->event_data_list.data[i];
        if (atoi(pevent_data->idx) == pnt) {
            return pevent_data;
        }
    }

    return NULL;
};

unsigned long g_MsToNTP[1000] =
{
	0x00000000, 0x00418937, 0x0083126f, 0x00c49ba6, 0x010624dd, 0x0147ae14,
	0x0189374c, 0x01cac083, 0x020c49ba, 0x024dd2f2, 0x028f5c29, 0x02d0e560,
	0x03126e98, 0x0353f7cf, 0x03958106, 0x03d70a3d, 0x04189375, 0x045a1cac,
	0x049ba5e3, 0x04dd2f1b, 0x051eb852, 0x05604189, 0x05a1cac1, 0x05e353f8,
	0x0624dd2f, 0x06666666, 0x06a7ef9e, 0x06e978d5, 0x072b020c, 0x076c8b44,
	0x07ae147b, 0x07ef9db2, 0x083126e9, 0x0872b021, 0x08b43958, 0x08f5c28f,
	0x09374bc7, 0x0978d4fe, 0x09ba5e35, 0x09fbe76d, 0x0a3d70a4, 0x0a7ef9db,
	0x0ac08312, 0x0b020c4a, 0x0b439581, 0x0b851eb8, 0x0bc6a7f0, 0x0c083127,
	0x0c49ba5e, 0x0c8b4396, 0x0ccccccd, 0x0d0e5604, 0x0d4fdf3b, 0x0d916873,
	0x0dd2f1aa, 0x0e147ae1, 0x0e560419, 0x0e978d50, 0x0ed91687, 0x0f1a9fbe,
	0x0f5c28f6, 0x0f9db22d, 0x0fdf3b64, 0x1020c49c, 0x10624dd3, 0x10a3d70a,
	0x10e56042, 0x1126e979, 0x116872b0, 0x11a9fbe7, 0x11eb851f, 0x122d0e56,
	0x126e978d, 0x12b020c5, 0x12f1a9fc, 0x13333333, 0x1374bc6a, 0x13b645a2,
	0x13f7ced9, 0x14395810, 0x147ae148, 0x14bc6a7f, 0x14fdf3b6, 0x153f7cee,
	0x15810625, 0x15c28f5c, 0x16041893, 0x1645a1cb, 0x16872b02, 0x16c8b439,
	0x170a3d71, 0x174bc6a8, 0x178d4fdf, 0x17ced917, 0x1810624e, 0x1851eb85,
	0x189374bc, 0x18d4fdf4, 0x1916872b, 0x19581062, 0x1999999a, 0x19db22d1,
	0x1a1cac08, 0x1a5e353f, 0x1a9fbe77, 0x1ae147ae, 0x1b22d0e5, 0x1b645a1d,
	0x1ba5e354, 0x1be76c8b, 0x1c28f5c3, 0x1c6a7efa, 0x1cac0831, 0x1ced9168,
	0x1d2f1aa0, 0x1d70a3d7, 0x1db22d0e, 0x1df3b646, 0x1e353f7d, 0x1e76c8b4,
	0x1eb851ec, 0x1ef9db23, 0x1f3b645a, 0x1f7ced91, 0x1fbe76c9, 0x20000000,
	0x20418937, 0x2083126f, 0x20c49ba6, 0x210624dd, 0x2147ae14, 0x2189374c,
	0x21cac083, 0x220c49ba, 0x224dd2f2, 0x228f5c29, 0x22d0e560, 0x23126e98,
	0x2353f7cf, 0x23958106, 0x23d70a3d, 0x24189375, 0x245a1cac, 0x249ba5e3,
	0x24dd2f1b, 0x251eb852, 0x25604189, 0x25a1cac1, 0x25e353f8, 0x2624dd2f,
	0x26666666, 0x26a7ef9e, 0x26e978d5, 0x272b020c, 0x276c8b44, 0x27ae147b,
	0x27ef9db2, 0x283126e9, 0x2872b021, 0x28b43958, 0x28f5c28f, 0x29374bc7,
	0x2978d4fe, 0x29ba5e35, 0x29fbe76d, 0x2a3d70a4, 0x2a7ef9db, 0x2ac08312,
	0x2b020c4a, 0x2b439581, 0x2b851eb8, 0x2bc6a7f0, 0x2c083127, 0x2c49ba5e,
	0x2c8b4396, 0x2ccccccd, 0x2d0e5604, 0x2d4fdf3b, 0x2d916873, 0x2dd2f1aa,
	0x2e147ae1, 0x2e560419, 0x2e978d50, 0x2ed91687, 0x2f1a9fbe, 0x2f5c28f6,
	0x2f9db22d, 0x2fdf3b64, 0x3020c49c, 0x30624dd3, 0x30a3d70a, 0x30e56042,
	0x3126e979, 0x316872b0, 0x31a9fbe7, 0x31eb851f, 0x322d0e56, 0x326e978d,
	0x32b020c5, 0x32f1a9fc, 0x33333333, 0x3374bc6a, 0x33b645a2, 0x33f7ced9,
	0x34395810, 0x347ae148, 0x34bc6a7f, 0x34fdf3b6, 0x353f7cee, 0x35810625,
	0x35c28f5c, 0x36041893, 0x3645a1cb, 0x36872b02, 0x36c8b439, 0x370a3d71,
	0x374bc6a8, 0x378d4fdf, 0x37ced917, 0x3810624e, 0x3851eb85, 0x389374bc,
	0x38d4fdf4, 0x3916872b, 0x39581062, 0x3999999a, 0x39db22d1, 0x3a1cac08,
	0x3a5e353f, 0x3a9fbe77, 0x3ae147ae, 0x3b22d0e5, 0x3b645a1d, 0x3ba5e354,
	0x3be76c8b, 0x3c28f5c3, 0x3c6a7efa, 0x3cac0831, 0x3ced9168, 0x3d2f1aa0,
	0x3d70a3d7, 0x3db22d0e, 0x3df3b646, 0x3e353f7d, 0x3e76c8b4, 0x3eb851ec,
	0x3ef9db23, 0x3f3b645a, 0x3f7ced91, 0x3fbe76c9, 0x40000000, 0x40418937,
	0x4083126f, 0x40c49ba6, 0x410624dd, 0x4147ae14, 0x4189374c, 0x41cac083,
	0x420c49ba, 0x424dd2f2, 0x428f5c29, 0x42d0e560, 0x43126e98, 0x4353f7cf,
	0x43958106, 0x43d70a3d, 0x44189375, 0x445a1cac, 0x449ba5e3, 0x44dd2f1b,
	0x451eb852, 0x45604189, 0x45a1cac1, 0x45e353f8, 0x4624dd2f, 0x46666666,
	0x46a7ef9e, 0x46e978d5, 0x472b020c, 0x476c8b44, 0x47ae147b, 0x47ef9db2,
	0x483126e9, 0x4872b021, 0x48b43958, 0x48f5c28f, 0x49374bc7, 0x4978d4fe,
	0x49ba5e35, 0x49fbe76d, 0x4a3d70a4, 0x4a7ef9db, 0x4ac08312, 0x4b020c4a,
	0x4b439581, 0x4b851eb8, 0x4bc6a7f0, 0x4c083127, 0x4c49ba5e, 0x4c8b4396,
	0x4ccccccd, 0x4d0e5604, 0x4d4fdf3b, 0x4d916873, 0x4dd2f1aa, 0x4e147ae1,
	0x4e560419, 0x4e978d50, 0x4ed91687, 0x4f1a9fbe, 0x4f5c28f6, 0x4f9db22d,
	0x4fdf3b64, 0x5020c49c, 0x50624dd3, 0x50a3d70a, 0x50e56042, 0x5126e979,
	0x516872b0, 0x51a9fbe7, 0x51eb851f, 0x522d0e56, 0x526e978d, 0x52b020c5,
	0x52f1a9fc, 0x53333333, 0x5374bc6a, 0x53b645a2, 0x53f7ced9, 0x54395810,
	0x547ae148, 0x54bc6a7f, 0x54fdf3b6, 0x553f7cee, 0x55810625, 0x55c28f5c,
	0x56041893, 0x5645a1cb, 0x56872b02, 0x56c8b439, 0x570a3d71, 0x574bc6a8,
	0x578d4fdf, 0x57ced917, 0x5810624e, 0x5851eb85, 0x589374bc, 0x58d4fdf4,
	0x5916872b, 0x59581062, 0x5999999a, 0x59db22d1, 0x5a1cac08, 0x5a5e353f,
	0x5a9fbe77, 0x5ae147ae, 0x5b22d0e5, 0x5b645a1d, 0x5ba5e354, 0x5be76c8b,
	0x5c28f5c3, 0x5c6a7efa, 0x5cac0831, 0x5ced9168, 0x5d2f1aa0, 0x5d70a3d7,
	0x5db22d0e, 0x5df3b646, 0x5e353f7d, 0x5e76c8b4, 0x5eb851ec, 0x5ef9db23,
	0x5f3b645a, 0x5f7ced91, 0x5fbe76c9, 0x60000000, 0x60418937, 0x6083126f,
	0x60c49ba6, 0x610624dd, 0x6147ae14, 0x6189374c, 0x61cac083, 0x620c49ba,
	0x624dd2f2, 0x628f5c29, 0x62d0e560, 0x63126e98, 0x6353f7cf, 0x63958106,
	0x63d70a3d, 0x64189375, 0x645a1cac, 0x649ba5e3, 0x64dd2f1b, 0x651eb852,
	0x65604189, 0x65a1cac1, 0x65e353f8, 0x6624dd2f, 0x66666666, 0x66a7ef9e,
	0x66e978d5, 0x672b020c, 0x676c8b44, 0x67ae147b, 0x67ef9db2, 0x683126e9,
	0x6872b021, 0x68b43958, 0x68f5c28f, 0x69374bc7, 0x6978d4fe, 0x69ba5e35,
	0x69fbe76d, 0x6a3d70a4, 0x6a7ef9db, 0x6ac08312, 0x6b020c4a, 0x6b439581,
	0x6b851eb8, 0x6bc6a7f0, 0x6c083127, 0x6c49ba5e, 0x6c8b4396, 0x6ccccccd,
	0x6d0e5604, 0x6d4fdf3b, 0x6d916873, 0x6dd2f1aa, 0x6e147ae1, 0x6e560419,
	0x6e978d50, 0x6ed91687, 0x6f1a9fbe, 0x6f5c28f6, 0x6f9db22d, 0x6fdf3b64,
	0x7020c49c, 0x70624dd3, 0x70a3d70a, 0x70e56042, 0x7126e979, 0x716872b0,
	0x71a9fbe7, 0x71eb851f, 0x722d0e56, 0x726e978d, 0x72b020c5, 0x72f1a9fc,
	0x73333333, 0x7374bc6a, 0x73b645a2, 0x73f7ced9, 0x74395810, 0x747ae148,
	0x74bc6a7f, 0x74fdf3b6, 0x753f7cee, 0x75810625, 0x75c28f5c, 0x76041893,
	0x7645a1cb, 0x76872b02, 0x76c8b439, 0x770a3d71, 0x774bc6a8, 0x778d4fdf,
	0x77ced917, 0x7810624e, 0x7851eb85, 0x789374bc, 0x78d4fdf4, 0x7916872b,
	0x79581062, 0x7999999a, 0x79db22d1, 0x7a1cac08, 0x7a5e353f, 0x7a9fbe77,
	0x7ae147ae, 0x7b22d0e5, 0x7b645a1d, 0x7ba5e354, 0x7be76c8b, 0x7c28f5c3,
	0x7c6a7efa, 0x7cac0831, 0x7ced9168, 0x7d2f1aa0, 0x7d70a3d7, 0x7db22d0e,
	0x7df3b646, 0x7e353f7d, 0x7e76c8b4, 0x7eb851ec, 0x7ef9db23, 0x7f3b645a,
	0x7f7ced91, 0x7fbe76c9, 0x80000000, 0x80418937, 0x8083126f, 0x80c49ba6,
	0x810624dd, 0x8147ae14, 0x8189374c, 0x81cac083, 0x820c49ba, 0x824dd2f2,
	0x828f5c29, 0x82d0e560, 0x83126e98, 0x8353f7cf, 0x83958106, 0x83d70a3d,
	0x84189375, 0x845a1cac, 0x849ba5e3, 0x84dd2f1b, 0x851eb852, 0x85604189,
	0x85a1cac1, 0x85e353f8, 0x8624dd2f, 0x86666666, 0x86a7ef9e, 0x86e978d5,
	0x872b020c, 0x876c8b44, 0x87ae147b, 0x87ef9db2, 0x883126e9, 0x8872b021,
	0x88b43958, 0x88f5c28f, 0x89374bc7, 0x8978d4fe, 0x89ba5e35, 0x89fbe76d,
	0x8a3d70a4, 0x8a7ef9db, 0x8ac08312, 0x8b020c4a, 0x8b439581, 0x8b851eb8,
	0x8bc6a7f0, 0x8c083127, 0x8c49ba5e, 0x8c8b4396, 0x8ccccccd, 0x8d0e5604,
	0x8d4fdf3b, 0x8d916873, 0x8dd2f1aa, 0x8e147ae1, 0x8e560419, 0x8e978d50,
	0x8ed91687, 0x8f1a9fbe, 0x8f5c28f6, 0x8f9db22d, 0x8fdf3b64, 0x9020c49c,
	0x90624dd3, 0x90a3d70a, 0x90e56042, 0x9126e979, 0x916872b0, 0x91a9fbe7,
	0x91eb851f, 0x922d0e56, 0x926e978d, 0x92b020c5, 0x92f1a9fc, 0x93333333,
	0x9374bc6a, 0x93b645a2, 0x93f7ced9, 0x94395810, 0x947ae148, 0x94bc6a7f,
	0x94fdf3b6, 0x953f7cee, 0x95810625, 0x95c28f5c, 0x96041893, 0x9645a1cb,
	0x96872b02, 0x96c8b439, 0x970a3d71, 0x974bc6a8, 0x978d4fdf, 0x97ced917,
	0x9810624e, 0x9851eb85, 0x989374bc, 0x98d4fdf4, 0x9916872b, 0x99581062,
	0x9999999a, 0x99db22d1, 0x9a1cac08, 0x9a5e353f, 0x9a9fbe77, 0x9ae147ae,
	0x9b22d0e5, 0x9b645a1d, 0x9ba5e354, 0x9be76c8b, 0x9c28f5c3, 0x9c6a7efa,
	0x9cac0831, 0x9ced9168, 0x9d2f1aa0, 0x9d70a3d7, 0x9db22d0e, 0x9df3b646,
	0x9e353f7d, 0x9e76c8b4, 0x9eb851ec, 0x9ef9db23, 0x9f3b645a, 0x9f7ced91,
	0x9fbe76c9, 0xa0000000, 0xa0418937, 0xa083126f, 0xa0c49ba6, 0xa10624dd,
	0xa147ae14, 0xa189374c, 0xa1cac083, 0xa20c49ba, 0xa24dd2f2, 0xa28f5c29,
	0xa2d0e560, 0xa3126e98, 0xa353f7cf, 0xa3958106, 0xa3d70a3d, 0xa4189375,
	0xa45a1cac, 0xa49ba5e3, 0xa4dd2f1b, 0xa51eb852, 0xa5604189, 0xa5a1cac1,
	0xa5e353f8, 0xa624dd2f, 0xa6666666, 0xa6a7ef9e, 0xa6e978d5, 0xa72b020c,
	0xa76c8b44, 0xa7ae147b, 0xa7ef9db2, 0xa83126e9, 0xa872b021, 0xa8b43958,
	0xa8f5c28f, 0xa9374bc7, 0xa978d4fe, 0xa9ba5e35, 0xa9fbe76d, 0xaa3d70a4,
	0xaa7ef9db, 0xaac08312, 0xab020c4a, 0xab439581, 0xab851eb8, 0xabc6a7f0,
	0xac083127, 0xac49ba5e, 0xac8b4396, 0xaccccccd, 0xad0e5604, 0xad4fdf3b,
	0xad916873, 0xadd2f1aa, 0xae147ae1, 0xae560419, 0xae978d50, 0xaed91687,
	0xaf1a9fbe, 0xaf5c28f6, 0xaf9db22d, 0xafdf3b64, 0xb020c49c, 0xb0624dd3,
	0xb0a3d70a, 0xb0e56042, 0xb126e979, 0xb16872b0, 0xb1a9fbe7, 0xb1eb851f,
	0xb22d0e56, 0xb26e978d, 0xb2b020c5, 0xb2f1a9fc, 0xb3333333, 0xb374bc6a,
	0xb3b645a2, 0xb3f7ced9, 0xb4395810, 0xb47ae148, 0xb4bc6a7f, 0xb4fdf3b6,
	0xb53f7cee, 0xb5810625, 0xb5c28f5c, 0xb6041893, 0xb645a1cb, 0xb6872b02,
	0xb6c8b439, 0xb70a3d71, 0xb74bc6a8, 0xb78d4fdf, 0xb7ced917, 0xb810624e,
	0xb851eb85, 0xb89374bc, 0xb8d4fdf4, 0xb916872b, 0xb9581062, 0xb999999a,
	0xb9db22d1, 0xba1cac08, 0xba5e353f, 0xba9fbe77, 0xbae147ae, 0xbb22d0e5,
	0xbb645a1d, 0xbba5e354, 0xbbe76c8b, 0xbc28f5c3, 0xbc6a7efa, 0xbcac0831,
	0xbced9168, 0xbd2f1aa0, 0xbd70a3d7, 0xbdb22d0e, 0xbdf3b646, 0xbe353f7d,
	0xbe76c8b4, 0xbeb851ec, 0xbef9db23, 0xbf3b645a, 0xbf7ced91, 0xbfbe76c9,
	0xc0000000, 0xc0418937, 0xc083126f, 0xc0c49ba6, 0xc10624dd, 0xc147ae14,
	0xc189374c, 0xc1cac083, 0xc20c49ba, 0xc24dd2f2, 0xc28f5c29, 0xc2d0e560,
	0xc3126e98, 0xc353f7cf, 0xc3958106, 0xc3d70a3d, 0xc4189375, 0xc45a1cac,
	0xc49ba5e3, 0xc4dd2f1b, 0xc51eb852, 0xc5604189, 0xc5a1cac1, 0xc5e353f8,
	0xc624dd2f, 0xc6666666, 0xc6a7ef9e, 0xc6e978d5, 0xc72b020c, 0xc76c8b44,
	0xc7ae147b, 0xc7ef9db2, 0xc83126e9, 0xc872b021, 0xc8b43958, 0xc8f5c28f,
	0xc9374bc7, 0xc978d4fe, 0xc9ba5e35, 0xc9fbe76d, 0xca3d70a4, 0xca7ef9db,
	0xcac08312, 0xcb020c4a, 0xcb439581, 0xcb851eb8, 0xcbc6a7f0, 0xcc083127,
	0xcc49ba5e, 0xcc8b4396, 0xcccccccd, 0xcd0e5604, 0xcd4fdf3b, 0xcd916873,
	0xcdd2f1aa, 0xce147ae1, 0xce560419, 0xce978d50, 0xced91687, 0xcf1a9fbe,
	0xcf5c28f6, 0xcf9db22d, 0xcfdf3b64, 0xd020c49c, 0xd0624dd3, 0xd0a3d70a,
	0xd0e56042, 0xd126e979, 0xd16872b0, 0xd1a9fbe7, 0xd1eb851f, 0xd22d0e56,
	0xd26e978d, 0xd2b020c5, 0xd2f1a9fc, 0xd3333333, 0xd374bc6a, 0xd3b645a2,
	0xd3f7ced9, 0xd4395810, 0xd47ae148, 0xd4bc6a7f, 0xd4fdf3b6, 0xd53f7cee,
	0xd5810625, 0xd5c28f5c, 0xd6041893, 0xd645a1cb, 0xd6872b02, 0xd6c8b439,
	0xd70a3d71, 0xd74bc6a8, 0xd78d4fdf, 0xd7ced917, 0xd810624e, 0xd851eb85,
	0xd89374bc, 0xd8d4fdf4, 0xd916872b, 0xd9581062, 0xd999999a, 0xd9db22d1,
	0xda1cac08, 0xda5e353f, 0xda9fbe77, 0xdae147ae, 0xdb22d0e5, 0xdb645a1d,
	0xdba5e354, 0xdbe76c8b, 0xdc28f5c3, 0xdc6a7efa, 0xdcac0831, 0xdced9168,
	0xdd2f1aa0, 0xdd70a3d7, 0xddb22d0e, 0xddf3b646, 0xde353f7d, 0xde76c8b4,
	0xdeb851ec, 0xdef9db23, 0xdf3b645a, 0xdf7ced91, 0xdfbe76c9, 0xe0000000,
	0xe0418937, 0xe083126f, 0xe0c49ba6, 0xe10624dd, 0xe147ae14, 0xe189374c,
	0xe1cac083, 0xe20c49ba, 0xe24dd2f2, 0xe28f5c29, 0xe2d0e560, 0xe3126e98,
	0xe353f7cf, 0xe3958106, 0xe3d70a3d, 0xe4189375, 0xe45a1cac, 0xe49ba5e3,
	0xe4dd2f1b, 0xe51eb852, 0xe5604189, 0xe5a1cac1, 0xe5e353f8, 0xe624dd2f,
	0xe6666666, 0xe6a7ef9e, 0xe6e978d5, 0xe72b020c, 0xe76c8b44, 0xe7ae147b,
	0xe7ef9db2, 0xe83126e9, 0xe872b021, 0xe8b43958, 0xe8f5c28f, 0xe9374bc7,
	0xe978d4fe, 0xe9ba5e35, 0xe9fbe76d, 0xea3d70a4, 0xea7ef9db, 0xeac08312,
	0xeb020c4a, 0xeb439581, 0xeb851eb8, 0xebc6a7f0, 0xec083127, 0xec49ba5e,
	0xec8b4396, 0xeccccccd, 0xed0e5604, 0xed4fdf3b, 0xed916873, 0xedd2f1aa,
	0xee147ae1, 0xee560419, 0xee978d50, 0xeed91687, 0xef1a9fbe, 0xef5c28f6,
	0xef9db22d, 0xefdf3b64, 0xf020c49c, 0xf0624dd3, 0xf0a3d70a, 0xf0e56042,
	0xf126e979, 0xf16872b0, 0xf1a9fbe7, 0xf1eb851f, 0xf22d0e56, 0xf26e978d,
	0xf2b020c5, 0xf2f1a9fc, 0xf3333333, 0xf374bc6a, 0xf3b645a2, 0xf3f7ced9,
	0xf4395810, 0xf47ae148, 0xf4bc6a7f, 0xf4fdf3b6, 0xf53f7cee, 0xf5810625,
	0xf5c28f5c, 0xf6041893, 0xf645a1cb, 0xf6872b02, 0xf6c8b439, 0xf70a3d71,
	0xf74bc6a8, 0xf78d4fdf, 0xf7ced917, 0xf810624e, 0xf851eb85, 0xf89374bc,
	0xf8d4fdf4, 0xf916872b, 0xf9581062, 0xf999999a, 0xf9db22d1, 0xfa1cac08,
	0xfa5e353f, 0xfa9fbe77, 0xfae147ae, 0xfb22d0e5, 0xfb645a1d, 0xfba5e354,
	0xfbe76c8b, 0xfc28f5c3, 0xfc6a7efa, 0xfcac0831, 0xfced9168, 0xfd2f1aa0,
	0xfd70a3d7, 0xfdb22d0e, 0xfdf3b646, 0xfe353f7d, 0xfe76c8b4, 0xfeb851ec,
	0xfef9db23, 0xff3b645a, 0xff7ced91, 0xffbe76c9
};

unsigned long SNTP_MsToNtpFraction( unsigned short wMilliSeconds )
{
	if( wMilliSeconds < 1000 )
		return (g_MsToNTP[wMilliSeconds]);
	return 0;
}

void Get_ACSI_Timestamp(DEV_VAL_UPDATE_TIME *ts, void *p)
{
	if( p == NULL )
	{
        ts->choice = 1;
		struct timeval tv;
		struct timezone tz;
		gettimeofday(&tv, NULL);
        /* printf("min=%d %lu\n", tz.tz_minuteswest, tv.tv_sec); */
		/* if( tz.tz_minuteswest == 0 ) */
		/* 	tz.tz_minuteswest = -480; */
		unsigned short wMS = (unsigned short)(tv.tv_usec / 1000);
		ts->acsi_time.SecondSinceEpoch = tv.tv_sec+tz.tz_minuteswest*60;
		ts->acsi_time.FractionOfSecond = (SNTP_MsToNtpFraction(wMS)>>8) & 0x00ffffff;
		ts->acsi_time.TimeQuality = 0x0a;
	}
	else
	{
		memcpy(ts, p, sizeof(ACSI_TIMESTAMP));
	}
}

static int emu2000shm_get_event_val(OUTDATA *pdata, OUTVAL *pval)
{
    SHM_SPACE *psharem = (SHM_SPACE *)g_emu2000_shm_data.psharem;
    if (psharem == NULL) {
        OUT_LOG_WARN_BUF("emu2000shm_get_ai", "psharem is NULL");
        return ERR_PNULL;
    }
    DEV_VAL_UPDATE_TIME ts;
    Get_ACSI_Timestamp(&ts, NULL);

    /* 定初始位置 */
    static int aie_pos = -1;
    if (aie_pos == -1) {
        aie_pos = psharem->sysInfo.nVARLISTWritePos;
        return ERR_OK;
    }

    if (aie_pos == psharem->sysInfo.nVARLISTWritePos) {
        return ERR_OK;
    }
    printf("aie_pos=%d write_pos=%d\n", aie_pos,
           psharem->sysInfo.nVARLISTWritePos);

    OUT_LOG_DEBUG_BUF("emu2000shm_get_ai", "valslist_pos=%d write_pos=%d",
                      aie_pos, psharem->sysInfo.nVARLISTWritePos);

    VARSLIST *paie = &psharem->varslist[aie_pos];
    aie_pos += 1;
    aie_pos = aie_pos % VARIBLE_MAX_NUM;

    OUTVAL_EVENT_VALS *pevent = (OUTVAL_EVENT_VALS *)pval->struct_val.buf;
    if (pevent == NULL || pval->struct_val.len != sizeof(OUTVAL_EVENT_VALS)) {
        OUT_LOG_WARN_BUF("emu2000shm_get_ai",
                         "invalid struct_val %p len=%d %lu", pevent,
                         pval->struct_val.len, sizeof(OUTVAL_EVENT_VALS));
        return ERR_O_RANGE;
    }

    printf("aie: stn=%d num=%d\n", paie->wSerialNo, paie->num);

    char guid[32];
    snprintf(guid, sizeof(guid), "%d", paie->wSerialNo + 1);

    pevent->pdev = dev_list_data_byguid(guid);
    if (pevent->pdev == NULL) {
        OUT_LOG_WARN_BUF("emu2000shm_get_ai",
                         "dev_list_find_by_guid failed guid=%s", guid);
        return ERR_O_RANGE;
    }

    /* pevent->event_name = */
    /*     emu2000shm_get_name(pevent->pdev, EMU2000_DATA_TYPE_YC, paie->wPntNum); */
    /* if (pevent->event_name == NULL) { */
    /*     OUT_LOG_WARN_BUF("emu2000shm_get_ai", */
    /*                      "emu2000shm_get_name failed guid=%s type=%s pnt=%d", */
    /*                      guid, EMU2000_DATA_TYPE_YC, paie->wPntNum); */
    /*     return ERR_O_RANGE; */
    /* } */

    int i;
    IEC61850_CFG_TEMPLATE_EVENT_DATA_LIST *ptlist =
        &pevent->pdev->ptemplate->event_data_list;

    int num = 0;
    int list_idx = 0;
    if (paie->num < 1) {
        return ERR_O_RANGE;
    }
    else {
        VARIABLE *pval = &paie->vars[1];
        /* 在定义的PX25和PX20中，数据2为Primary,以Primary 来确定是告警还是脱扣 */
        if (pval->iType == VARIABLE_TYPE_INT) {
            /* 4为告警，3为脱扣，与配置软件中作对应 */
            /* if (pval->iVal == 4) { */
            /*     pevent->event_name = ptlist->data[0].name; */
            /*     num = ptlist->data[0].num; */
            /*     list_idx = 0; */
            /* } */
            /* 4为告警，3为脱扣，与配置软件中作对应 */
            if (pval->iVal == 3) {
                pevent->event_name = ptlist->data[1].name;
                num = ptlist->data[1].num;
                list_idx = 1;
            }
            else if(pval->iVal == 1 || pval->iVal == 2 || pval->iVal == 4){
                pevent->event_name = ptlist->data[0].name;
                num = ptlist->data[0].num;
                list_idx = 0;
            }
            else {
                OUT_LOG_WARN_BUF("emu2000shm_get_ai", "invalid Primary val=%d",
                                 pval->iVal);
                return ERR_O_RANGE;
            }
        }
    }

    IEC61850_CFG_TEMPLATE_EVENT_DATA *pevent_data_list =
        &ptlist->data[list_idx];

    pevent->num = num;

    uint64_t event_utc_time = 0;
    for (i = 0; i < num; i++) {
        IEC61850_CFG_TEMPLATE_EVENT_DATA_ITEM *pevent_data =
            &pevent_data_list->items[i];
        int idx = atoi(pevent_data->idx);
        if (idx <= 0 || idx > num) {
            OUT_LOG_WARN_BUF("emu2000shm_get_ai",
                             "event_data_list idx not match num=%d expected=%d",
                             idx, num);
            return ERR_O_RANGE;
        }

        VARIABLE *pval = &paie->vars[idx - 1];
        if (0 == strcmp(pevent_data->type, "float")) {
            pevent->val[i].type = DVT_FLOAT;
            pevent->val[i].fval =
                ((pval->iType == VARIABLE_TYPE_FLOAT) ? pval->fVal :
                 (pval->iType == VARIABLE_TYPE_INT)   ? pval->iVal :
                                                        pval->uiVal);

            OUT_LOG_DEBUG_BUF("emu2000shm_get_ai", "val[%d] type=%d val=%f", i,
                              pval->iType, pevent->val[i].fval);
        }
        else if (0 == strcmp(pevent_data->type, "int")) {
            pevent->val[i].type = DVT_INT;
            pevent->val[i].ival =
                ((pval->iType == VARIABLE_TYPE_FLOAT) ? (int)pval->fVal :
                 (pval->iType == VARIABLE_TYPE_INT)   ? pval->iVal :
                                                        (int)pval->uiVal);
            OUT_LOG_DEBUG_BUF("emu2000shm_get_ai", "val[%d] type=%d val=%d", i,
                              pval->iType, pevent->val[i].ival);
        }
        else if (0 == strcmp(pevent_data->type, "uint")) {
            pevent->val[i].type = DVT_DWORD;
            pevent->val[i].doval =
                ((pval->iType == VARIABLE_TYPE_FLOAT) ?
                     (unsigned int)pval->fVal :
                 (pval->iType == VARIABLE_TYPE_INT) ? (unsigned int)pval->iVal :
                                                      pval->uiVal);
            OUT_LOG_DEBUG_BUF("emu2000shm_get_ai", "val[%d] type=%d val=%u", i,
                              pval->iType, pevent->val[i].doval);
        }
        else if (0 == strcmp(pevent_data->type, "utc")) {
            pevent->val[i].type = DVT_UTCTIME;
            pevent->val[i].utc_time =
                ((pval->iType == VARIABLE_TYPE_FLOAT) ?
                     (unsigned int)pval->fVal :
                 (pval->iType == VARIABLE_TYPE_INT) ? (unsigned int)pval->iVal :
                                                      pval->uiVal);

            pevent->val[i].utc_time *= 1000; /* do的时间单位是秒，转换为毫秒 */
            /* pevent->val[i].utc_time += (gs_diffutc_ms); */

            OUT_LOG_DEBUG_BUF("emu2000shm_get_ai",
                              "val[%d] type=%d utc_time_ms=%llu", i,
                              pval->iType, pevent->val[i].utc_time);
            event_utc_time = pevent->val[i].utc_time;
        }
        else {
            OUT_LOG_WARN_BUF("emu2000shm_get_ai",
                             "unsupported event_data type=%s",
                             pevent_data->type);
            return ERR_TYPE;
        }
    }

    for (i = 0; i < num; i++) {
        if (event_utc_time > 0 ) {
            pevent->val[i].t.choice = 2;
            pevent->val[i].t.utc_time = event_utc_time;
        }
        else {
            Get_ACSI_Timestamp(&pevent->val[i].t, NULL);
        }

    }

    OUT_LOG_INFO_BUF("emu2000shm_get_ai",
                     "Get AIE event %p guid=%s event=%s type=%d num=%d", pevent,
                     guid, pevent->event_name, paie->wSerialNo, num);

    return 1;
};

int emu2000shm_get_val(OUTDATA *pdata, OUTVAL *pval)
{
    int mtype = pval->type & 0xFFFF0000;
    switch (mtype) {
    case OVM_DEVTYPE: {
        return emu2000shm_get_realtime_data(pdata, pval);
    } break;
    case OVM_REALTIME: {
        return emu2000shm_get_realtime_data(pdata, pval);
    } break;
    case OVM_DEVDATA: {
        switch (pval->ntype) {
        case OVT_PORT_ADDR: {
            return emu2000shm_get_guid(pdata, pval);
        } break;
        case OVT_DEV_COMMSTATE: {
            return emu2000shm_get_commstate(pdata, pval);
        } break;
        default: {
            OUT_LOG_WARN_BUF("emu2000shm_get_val", "unsupported type=%x",
                             pval->ntype);
            return ERR_TYPE;
        }
        }

    } break;
    case OVM_RECORD: {
        switch (pval->ntype) {
        case OVT_EVENT: {
            return emu2000shm_get_soe(pdata, pval);
        } break;
        case OVT_EVENT_VALS: {
            return emu2000shm_get_event_val(pdata, pval);
        } break;
        }
    } break;
    default:
        OUT_LOG_WARN_BUF("emu2000shm_get_val", "unsupported mtype=%x", mtype);
        break;
    }

    return ERR_INVAL;
}

int emu2000shm_get_list_val(OUTDATA *pdata, OUTVAL_LIST *pval)
{
    if (pval == NULL || pval->vals == NULL || pval->num <= 0) {
        OUT_LOG_WARN_BUF("emu2000shm_get_list_val", "invalid pval");
        return ERR_O_RANGE;
    }

    int i;
    for (i = 0; i < pval->num; i++) {
        OUTVAL *pcur_val = &pval->vals[i];
        int ret = emu2000shm_get_val(pdata, pcur_val);
        if (ret != ERR_OK) {
            OUT_LOG_WARN_BUF("emu2000shm_get_list_val",
                             "emu2000shm_get_val failed ret=%d", ret);
            return ret;
        }
    }

    return ERR_OK;
}

void ShowRTDBInfo()
{
    OUT_LOG_INFO_BUF("ShowRTDBInfo", "********* RTDB Infomation *********");
    SHM_SPACE *psharem = (SHM_SPACE *)g_emu2000_shm_data.psharem;
    VARSLIST *pItem = &psharem->varslist[0];
    printf("VarList : %d\n", pItem->num);
    if (psharem) {
        OUT_LOG_INFO_BUF("ShowRTDBInfo", "Memory Address : %p", psharem);
        if (psharem) {
            OUT_LOG_INFO_BUF("ShowRTDBInfo", "All Memory Length  : %d",
                             (int)psharem->dwAllSize);
            OUT_LOG_INFO_BUF("ShowRTDBInfo", "Ext Memory Length  : %d",
                             (int)psharem->dwExtSize);
            OUT_LOG_INFO_BUF("ShowRTDBInfo", "The Space Quality  : %X",
                             (unsigned int)psharem->dwQuality);
            OUT_LOG_INFO_BUF("ShowRTDBInfo", "RTDBase State  : %d",
                             psharem->sysInfo.wState);
        }
        else {
            OUT_LOG_WARN_BUF("ShowRTDBInfo", "Invalid Memory Space !");
        }

        SYSINFO *pinfo = &psharem->sysInfo;
        OUT_LOG_INFO_BUF("ShowRTDBInfo", "Station Amount = %d", pinfo->wStnSum);
        OUT_LOG_INFO_BUF("ShowRTDBInfo", "Analog  Sum = %d", pinfo->nAnalogSum);
        OUT_LOG_INFO_BUF("ShowRTDBInfo", "Digital Sum = %d",
                         pinfo->nDigitalSum);
        OUT_LOG_INFO_BUF("ShowRTDBInfo", "Pulse   Sum = %d", pinfo->nPulseSum);
        OUT_LOG_INFO_BUF("ShowRTDBInfo", "Relay   Sum = %d", pinfo->nRelaySum);
        OUT_LOG_INFO_BUF("ShowRTDBInfo", "DZ      Sum = %d", pinfo->nAdjustSum);

        int i;
        for (i = 0; i < psharem->sysInfo.wStnSum; i++) {
            STNBUS_ADDR *pbusaddr = &psharem->RTDBase.StnBusAddr[i];
            OUT_LOG_INFO_BUF("ShowRTDBInfo", "Stn %02d : BusNo=%d DevAddr=%d",
                             i + 1, pbusaddr->byBusNo, pbusaddr->wDevAddr);

            STNPARAM *pStnObj = &psharem->RTDBase.StnUnit[i];
            OUT_LOG_INFO_BUF("ShowRTDBInfo",
                             "Stn %02d : name=%s Analog=%d Digital=%d "
                             "Pulse=%d Relay=%d DZ=%d",
                             i + 1, pStnObj->szStnName, pStnObj->wAnalogSum,
                             pStnObj->wDigitalSum, pStnObj->wPulseSum,
                             pStnObj->wRelaySum, pStnObj->wDZSum);
        }
    }
    else {
        OUT_LOG_WARN_BUF("ShowRTDBInfo", "Invalid RTDB Object !");
    }
    OUT_LOG_INFO_BUF("ShowRTDBInfo", "----All Copy Right By ESD ----");
}

int emu2000shm_init(OUTDATA *pdata)
{
    /* ini_parse(EMU2000_BUSLINE_INI_NAME, ntpclient_info_handle, */
    /*                     &gs_timezone); */
    /* gs_diffutc_ms = -gs_timezone * 60 * 60 * 1000; */
    /* printf("gs_timezone=%d\n", gs_timezone); */

    int ret = sharem_open(&g_emu2000_shm_data, g_emu2000_shm_data.sharem_id,
                          g_emu2000_shm_data.size, 0600);
    if (ret < 0) {
        OUT_LOG_WARN_BUF("emu2000shm_init", "sharem_open failed");
        return -1;
    }

    void *p = sharem_attach(&g_emu2000_shm_data);
    if (p == NULL) {
        OUT_LOG_WARN_BUF("emu2000shm_init", "sharem_attach failed");
        return -2;
    }

    SHM_SPACE *psharem = (SHM_SPACE *)g_emu2000_shm_data.psharem;
    /* for (int i = 0; i < psharem->sysInfo.wStnSum; i++) { */
    /*     STNPARAM *pStnObj = &psharem->RTDBase.StnUnit[i]; */
    /*     pStnObj->pAnalogTab = */
    /*         &psharem->RTDBase.AnalogTable[pStnObj->dwAnalogPos]; */
    /*     pStnObj->pDigitalTab = */
    /*         &psharem->RTDBase.DigitalTable[pStnObj->dwDigitalPos]; */
    /*     pStnObj->pRelayTab = &psharem->RTDBase.RelayTable[pStnObj->dwRelayPos]; */
    /*     pStnObj->pPulseTab = &psharem->RTDBase.PulseTable[pStnObj->dwPulsePos]; */
    /*     pStnObj->pDzTab = &psharem->RTDBase.DzTable[pStnObj->dwDZPos]; */

    /*     printf("Stn %02d : name=%s Analog=%d Digital=%d " */
    /*            "Pulse=%d Relay=%d DZ=%d, AnalogPos=%d DigitalPos=%lu " */
    /*            "RelayPos=%lu PulsePos=%lu DZPos=%lu\n", */
    /*            i + 1, pStnObj->szStnName, pStnObj->wAnalogSum, */
    /*            pStnObj->wDigitalSum, pStnObj->wPulseSum, pStnObj->wRelaySum, */
    /*            pStnObj->wDZSum, pStnObj->dwAnalogPos, pStnObj->dwDigitalPos, */
    /*            pStnObj->dwRelayPos, pStnObj->dwPulsePos, pStnObj->dwDZPos); */
    /* } */

    ShowRTDBInfo();

    pdata->get_val = emu2000shm_get_val;
    pdata->priv_data = &g_emu2000shm_priv_data;

    /* /\* VALSLIST *pval_list = &pdata->val_list; *\/ */
    /* VARSLIST varslist; */
    /* varslist.num = 1; */

    /* varslist.wSerialNo = 1; */
    /* int nPos = psharem->sysInfo.nSOEWritePos; */
    /* VARSLIST *pItem = &psharem->varslist[nPos]; */
    /* memcpy(pItem, &varslist, sizeof(VARSLIST)); */
    /* // printf( "WritePos = %d\n" , nPos ) ; */
    /* nPos = (nPos + 1) % VARSLIST_MAX_NUM; */
    /* psharem->sysInfo.nVARLISTWritePos = nPos; */

    OUT_LOG_INFO_BUF("emu2000shm_init", "ok shm=%p", p);

    return 0;
}

int emu2000shm_exit(OUTDATA *pdata)
{
    sharem_deattach(&g_emu2000_shm_data);
    pdata->priv_data = NULL;
    return 0;
}
