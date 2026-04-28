#include <pthread.h>
#include <unistd.h>
#include <stdbool.h>

#include "iec61850_macro.h"
#include "iec61850_server.h"

#include "err_def.h"
#include "log_conf.h"

#include "ied_data.h"
#include "svc_61850_server.h"
#include "led.h"

static int gs_svc_61850_server_thread_run = 0;
static pthread_t gs_svc_61850_server_thread_id = 0;

static void *svc_61850_server_thread(void *arg)
{
    IedServer server = ied_server();
    if (server == NULL) {
        log_error(SVCLOG, "IedServer is NULL!");
        gs_svc_61850_server_thread_id = 0;
        return NULL;
    }

    gs_svc_61850_server_thread_run = 1;
    int lcount = 0;

    while (gs_svc_61850_server_thread_run) {
        sleep(1);
        bool b = IedServer_isRunning(server);
        if (b) {
            log_debug(SVCLOG, "IedServer is running...");

            /* int count = IedServer_getNumberOfOpenConnections(server); */
            /* log_debug(SVCLOG, "Client connections: %d", count); */
            /* if (count > 0) { */
            /*     if (lcount == 0) { */
            /*         led_blink(LED_61850, LED_61850_BLINK_INTERVAL_MS); */
            /*     } */
            /* } */
            /* else { */
            /*     if (lcount > 0) { */
            /*         led_off(LED_61850); */
            /*     } */
            /* } */
            /* lcount = count; */
        }
        else {
            log_warn(SVCLOG, "IedServer is not running!");

            int tcp_port = 102;
            server = ied_server();
            if (server) {
                IedServer_start(server, tcp_port);
                log_info(SVCLOG, "IedServer started on port %d", tcp_port);
            }
        }
    }

    gs_svc_61850_server_thread_id = 0;
    return NULL;
}

int svc_61850_server_init(void)
{
    int tcp_port = 102;
    /* 初始化服务 */
    /* IedModel_setIedNameForDynamicModel(model, "test"); */
    IedServer server = ied_server();
    if (server) {
        IedServer_start(server, tcp_port);
        log_info(SVCLOG, "IedServer started on port %d", tcp_port);
    }

    return ERR_OK;

err:
    return -1;
}
int svc_61850_server_exit(void)
{
    return ERR_OK;
}
int svc_61850_server_run(void)
{
    if (gs_svc_61850_server_thread_id > 0) {
        log_info(SVCLOG, "svc_61850_server_thread is running");
        return ERR_OK;
    }

    if (gs_svc_61850_server_thread_run == 1) {
        log_info(SVCLOG, "svc_61850_server_thread is running");
        return ERR_OK;
    }

    pthread_create(&gs_svc_61850_server_thread_id, NULL,
                   svc_61850_server_thread, (void *)NULL);
    pthread_detach(gs_svc_61850_server_thread_id);

    return ERR_OK;
}

int svc_61850_server_stop(void)
{
    IedServer server = ied_server();
    if (server) {
        IedServer_stop(server);
        log_info(SVCLOG, "IedServer stop");
    }

    if (gs_svc_61850_server_thread_run == 1) {
        gs_svc_61850_server_thread_run = 0;
    }
    sleep(1);

    if (gs_svc_61850_server_thread_id > 0) {
        pthread_cancel(gs_svc_61850_server_thread_id);
    }

    sleep(1);

    return ERR_OK;
}
