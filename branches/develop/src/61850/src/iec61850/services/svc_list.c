#include "svc_list.h"
#include "svc_61850_server.h"
#include "svc_devlist_info.h"
#include "svc_timer_update_ied.h"
#include "svc_update_event.h"

static SVC_DATA svcs[] = {
    {
        .name = "svc_61850_server",

        .init = svc_61850_server_init,
        .exit = svc_61850_server_exit,
        .run = svc_61850_server_run,
        .stop = svc_61850_server_stop,
    },
    {
        .name = "svc_devlist_Info",

        .init = svc_devlist_info_init,
        .exit = svc_devlist_info_exit,
        .run = svc_devlist_info_run,
        .stop = svc_devlist_info_stop,
    },
    {
        .name = "svc_timer_update_ied",

        .init = svc_timer_update_ied_init,
        .exit = svc_timer_update_ied_exit,
        .run = svc_timer_update_ied_run,
        .stop = svc_timer_update_ied_stop,
    },
    {
        .name = "svc_update_event",
        .init = svc_update_event_init,
        .exit = svc_update_event_exit,
        .run = svc_update_event_run,
        .stop = svc_update_event_stop,
    },
};

int svc_list_init(void)
{
    for (int i = 0; i < sizeof(svcs) / sizeof(svcs[0]); i++) {
        SVC_DATA *psvc = &svcs[i];
        if (psvc->init != NULL) {
            int ret = psvc->init();
            if (ret != 0) {
                return ret;
            }
        }
    }
    return 0;
}
int svc_list_exit(void)
{
    for (int i = 0; i < sizeof(svcs) / sizeof(svcs[0]); i++) {
        SVC_DATA *psvc = &svcs[i];
        if (psvc->exit != NULL) {
            int ret = psvc->exit();
            if (ret != 0) {
                return ret;
            }
        }
    }
    return 0;
}

int svc_list_run(void)
{
    for (int i = 0; i < sizeof(svcs) / sizeof(svcs[0]); i++) {
        SVC_DATA *psvc = &svcs[i];
        if (psvc->run != NULL) {
            int ret = psvc->run();
            if (ret != 0) {
                return ret;
            }
        }
    }
    return 0;
}

int svc_list_stop(void)
{
    for (int i = 0; i < sizeof(svcs) / sizeof(svcs[0]); i++) {
        SVC_DATA *psvc = &svcs[i];
        if (psvc->stop != NULL) {
            int ret = psvc->stop();
            if (ret != 0) {
                return ret;
            }
        }
    }
    return 0;
}
