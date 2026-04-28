#include "err_def.h"
#include "outdata.h"
#include "emu2000shm.h"
#include "log_conf.h"

#include "unistd.h"

#ifdef OUTDATA_TYPE_EMU2000_SHM
OUTDATA gs_outdata[] = { {
    .type = ODT_EMU2000_SHM,
} };
#elif defined(OUTDATA_TYPE_SCU_JSON)
OUTDATA gs_outdata[] = { {
    .type = ODT_SCU_JSON,
} };
#else
OUTDATA gs_outdata[] = { {
    .type = ODT_EMU2000_SHM,
} };
#endif

OUTDATA *outdata_get(int type)
{
    for (int i = 0; i < sizeof(gs_outdata) / sizeof(OUTDATA); i++) {
        OUTDATA *pdata = &gs_outdata[i];
        if (pdata->type == type) {
            return pdata;
        }
    }

    return NULL;
}

int outdata_init()
{
    int i;
    for (i = 0; i < sizeof(gs_outdata) / sizeof(OUTDATA); i++) {
        OUTDATA *pdata = &gs_outdata[i];
        switch (pdata->type) {
        case ODT_EMU2000_SHM: {
            log_info(DVALOG, "outdata_init emu2000shm_init");
            int ret = emu2000shm_init(pdata);
            while (ret < 0) {
                log_error(DVALOG, "outdata_init emu2000shm_init failed ret=%d",
                          ret);
                sleep(1);
                log_info(DVALOG, "outdata_init retry emu2000shm_init");
                ret = emu2000shm_init(pdata);
            }

            /* OUTVAL val; */
            /* if (pdata->get_val != NULL) { */
            /*     val.type = OVM_DEVTYPE; */
            /*     ret = pdata->get_val(pdata, &val); */
            /*     while (ret <= 0) { */
            /*         log_error(DVALOG, */
            /*                   "outdata_init emu2000shm_get_val failed " */
            /*                   "ret=%d", */
            /*                   ret); */
            /*         val.type = OVM_DEVTYPE; */
            /*         ret = pdata->get_val(pdata, &val); */
            /*     } */
            /* } */

            return ret;
        } break;
        case ODT_SCU_JSON: {
            return 0;
        } break;
        default:
            break;
        }

        return -2;
    }

    return ERR_OK;
}
int outdata_exit()
{
    for (int i = 0; i < sizeof(gs_outdata) / sizeof(OUTDATA); i++) {
        OUTDATA *pdata = &gs_outdata[i];
        switch (pdata->type) {
        case ODT_EMU2000_SHM: {
            emu2000shm_exit(pdata);
        } break;
        case ODT_SCU_JSON: {
        } break;
        default:
            break;
        }

        return -2;
    }

    return ERR_OK;
}
