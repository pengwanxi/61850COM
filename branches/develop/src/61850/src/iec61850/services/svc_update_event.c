#include <pthread.h>
#include <string.h>
#include <unistd.h>
#include <stdbool.h>

#include "err_def.h"
#include "log_conf.h"

#include "svc_update_event.h"
#include "dev_data.h"
#include "dev_ied.h"
#include "dev_list.h"
#include "dev_val.h"
#include "outdata.h"
#include "outval.h"

#include "iec61850_conf.h"
extern IEC61850_CONF *g_pcfg;

static bool gs_svc_update_event_active = false;
static pthread_t gs_svc_update_event_thread_id = 0;

/**
 *  \brief 获取event 数据
 *  \param pied
 *  \return int
 */
static int svc_update_event(void)
{
    DEV_DATA *pied = dev_list_ied_data();
    if (pied == NULL) {
        printf("svc_update_event dev_list_data ied failed\n");
        return -1;
    }
    int ret = 0;

    OUTVAL_EVENT event;
    memset(&event, 0, sizeof(event));
    ret = outval_get_event(&event);
    while (ret > 0) {
        if (event.attr == 0) {
            log_warn(SVCLOG,
                     "svc_update_event invalid event "
                     "%s attr=0(CHGYX)",
                     event.event_name);
            break;
        }

        log_info(SVCLOG,
                 "svc_update_event get event %p guid=%s event=%s "
                 "state=%d time=%lu %lu",
                 &event, event.pdev->guid, event.event_name, event.bval,
                 event.time.utc_time / 1000, event.time.utc_time % 1000);

        DEV_VAL_EVENT dev_event;
        memset(&dev_event, 0, sizeof(dev_event));

        if (event.pdev == NULL || event.event_name == NULL) {
            log_warn(SVCLOG, "svc_update_event invalid event pdev "
                             "or event_name");
            break;
        }

        IEC61850_CFG_TEMPLATE *ptemplate = event.pdev->ptemplate;
        IEC61850_CFG_TEMPLATE_EVENT_LIST *pevent_list = &ptemplate->event_list;
        IEC61850_CFG_TEMPLATE_EVENT *pevent = NULL;

        for (int i = 0; i < pevent_list->num; i++) {
            IEC61850_CFG_TEMPLATE_EVENT *p = &pevent_list->event[i];
            if (0 == strcmp(p->name, event.event_name)) {
                pevent = p;
                break;
            }
        }
        if (pevent == NULL) {
            log_warn(SVCLOG,
                     "svc_update_event_thread event %s not found in "
                     "template %s",
                     event.event_name, ptemplate->name);
            break;
        }

        snprintf(dev_event.ref_name, sizeof(dev_event.ref_name), "%s%s/%s",
                 pied->devname, event.pdev->devname, pevent->ref_name);
        snprintf(dev_event.ref_time, sizeof(dev_event.ref_time), "%s%s/%s",
                 pied->devname, event.pdev->devname, pevent->ref_time);

        if (pevent->ref_q != NULL && strlen(pevent->ref_q) > 2) {
            snprintf(dev_event.ref_q, sizeof(dev_event.ref_q), "%s%s/%s",
                     pied->devname, event.pdev->devname, pevent->ref_q);
            dev_event.ref_q_valid = true;
        }
        else {
            memset(dev_event.ref_q, 0, sizeof(dev_event.ref_q));
            dev_event.ref_q_valid = false;
        }
        dev_event.state = event.bval;
        dev_event.utc_time = event.time.utc_time;

        DEV_VAL val;
        memset(&val, 0, sizeof(val));
        val.type = DVT_EVENT;
        val.event = &dev_event;

        log_info(SVCLOG, "dev_ied_set_val name=%s %s %s(%d) type=%d",
                 dev_event.ref_name, dev_event.ref_time, dev_event.ref_q,
                 dev_event.ref_q_valid, val.type);

        pied->set_record(pied, &val);
        memset(&event, 0, sizeof(event));
        ret = outval_get_event(&event);
    }

    return ret;
}

/**
 *  \brief 获取event 数据
 *  \param pied
 *  \return int
 */
static int svc_update_event_vals(void)
{
    DEV_DATA *pied = dev_list_ied_data();
    if (pied == NULL) {
        printf("svc_update_event_thread dev_list_data ied failed\n");
        return -1;
    }
    int ret = 0;

    OUTVAL_EVENT_VALS event_vals;
    memset(&event_vals, 0, sizeof(event_vals));
    ret = outval_get_event_vals(&event_vals);
    while (ret > 0) {
        DEV_VAL_EVENT dev_event;
        memset(&dev_event, 0, sizeof(dev_event));

        log_info(SVCLOG,
                 "svc_update_event_vals get event vals %p guid=%s event=%s "
                 "num=%d",
                 &event_vals, event_vals.pdev->guid, event_vals.event_name,
                 event_vals.num);

        if (event_vals.pdev == NULL || event_vals.event_name == NULL) {
            log_warn(SVCLOG, "svc_update_event_thread invalid event pdev "
                             "or event_name");
            break;
        }

        IEC61850_CFG_TEMPLATE *ptemplate = event_vals.pdev->ptemplate;
        IEC61850_CFG_TEMPLATE_EVENT_DATA_LIST *pevent_data_list =
            &ptemplate->event_data_list;
        IEC61850_CFG_TEMPLATE_EVENT_DATA *pevent = NULL;

        for (int i = 0; i < pevent_data_list->num; i++) {
            IEC61850_CFG_TEMPLATE_EVENT_DATA *p = &pevent_data_list->data[i];
            if (0 == strcmp(p->name, event_vals.event_name)) {
                pevent = p;
                break;
            }
        }
        if (pevent == NULL) {
            log_warn(SVCLOG,
                     "svc_update_event_thread event %s not found in "
                     "template %s",
                     event_vals.event_name, ptemplate->name);
            break;
        }

        for (int i = 0; i < event_vals.num && i < pevent->num; i++) {
            DEV_VAL val;
            memset(&val, 0, sizeof(val));

            char refname[256];
            snprintf(refname, sizeof(dev_event.ref_name), "%s/%s",
                     event_vals.pdev->devname, pevent->items[i].ref_name);
            snprintf(refname, sizeof(dev_event.ref_name), "%s/%s",
                     event_vals.pdev->devname, pevent->items[i].ref_name);
            memcpy(&val, &event_vals.val[i], sizeof(DEV_VAL));
            val.refname = refname;
            if (pevent->items[i].ref_time != NULL &&
                strlen(pevent->items[i].ref_time) > 0) {
                char reftime[256];
                snprintf(reftime, sizeof(reftime), "%s/%s",
                         event_vals.pdev->devname, pevent->items[i].ref_time);
                val.ref_time = reftime;
            }
            if (pevent->items[i].ref_q != NULL &&
                strlen(pevent->items[i].ref_q) > 2) {
                char refq[256];
                snprintf(refq, sizeof(refq), "%s/%s", event_vals.pdev->devname,
                         pevent->items[i].ref_q);
                val.ref_q = refq;
            }
            printf("svc_update_event_vals set val name=%s ref_time=%s ref_q=%s\n",
                   val.refname, val.ref_time, val.ref_q);
            pied->set_val(pied, &val);
        }

        memset(&event_vals, 0, sizeof(event_vals));
        ret = outval_get_event_vals(&event_vals);
    }

    return ret;
}

static void *svc_update_event_thread(void *arg)
{
    DEV_DATA *pied = dev_list_ied_data();
    if (pied == NULL) {
        printf("svc_update_event_thread dev_list_data ied failed\n");
        return NULL;
    }
    int ret = 0;

#if 0
    OUTVAL_EVENT_TIME_T t = {
        .utc_time = 1770692334000,
    };

    DEV_DATA *ptdev = NULL;
    for (int i = 0; i < 3; i++) {
        IEC61850_CFG_DEV *pcdev = &g_pcfg->devlist.devs[i];
        ptdev = dev_list_data(pcdev->devname);
        if (ptdev != NULL) {
            break;
        }
    }
    if (ptdev == NULL) {
        printf("svc_update_event_thread dev_list_data failed\n");
        return NULL;
    }
    OUTVAL_EVENT tevent = {
        .pdev = ptdev,
        .event_name = "OpenCloseState",
        .bval = 1,
    };

    DEV_VAL_EVENT tdevent;

    IEC61850_CFG_TEMPLATE *ptemplate = ptdev->ptemplate;
    IEC61850_CFG_TEMPLATE_EVENT_LIST *pevent_list = &ptemplate->event_list;
    IEC61850_CFG_TEMPLATE_EVENT *pevent = NULL;

    for (int i = 0; i < pevent_list->num; i++) {
        IEC61850_CFG_TEMPLATE_EVENT *p = &pevent_list->event[i];
        if (0 == strcmp(p->name, tevent.event_name)) {
            pevent = p;
            break;
        }
    }

    snprintf(tdevent.ref_name, sizeof(tdevent.ref_name), "%s/%s",
             ptdev->devname, pevent->ref_name);
    snprintf(tdevent.ref_time, sizeof(tdevent.ref_name), "%s/%s",
             ptdev->devname, pevent->ref_time);
    tdevent.state = tevent.bval;
    tdevent.utc_time = t.utc_time;

    DEV_VAL val;
    memset(&val, 0, sizeof(val));
    val.type = DVT_EVENT;
    val.event = &tdevent;
    while (1) {
        sleep(1);
        if (tevent.bval) {
            tevent.bval = 0;
        }
        else {
            tevent.bval = 1;
        }
        tdevent.state = !tevent.bval;
        tdevent.utc_time += 1000;
        pied->set_record(pied, &val);
    }

#endif
    while (gs_svc_update_event_active) {
        usleep(50 * 1000);
        svc_update_event();
        svc_update_event_vals();
    }

    gs_svc_update_event_active = false;
    return NULL;
}

int svc_update_event_init(void)
{
    return ERR_OK;
}

int svc_update_event_exit(void)
{
    if (gs_svc_update_event_thread_id != 0) {
        pthread_cancel(gs_svc_update_event_thread_id);
        gs_svc_update_event_thread_id = 0;
    }
    return ERR_OK;
}

int svc_update_event_stop(void)
{
    gs_svc_update_event_active = false;
    return ERR_OK;
}

int svc_update_event_run(void)
{
    if (gs_svc_update_event_active) {
        log_info(SVCLOG, "svc_update_event_trigger already processing");
        return ERR_OK;
    }

    gs_svc_update_event_active = true;

    if (gs_svc_update_event_thread_id != 0) {
        pthread_cancel(gs_svc_update_event_thread_id);
    }

    pthread_create(&gs_svc_update_event_thread_id, NULL,
                   svc_update_event_thread, NULL);
    pthread_detach(gs_svc_update_event_thread_id);
    return ERR_OK;
}
