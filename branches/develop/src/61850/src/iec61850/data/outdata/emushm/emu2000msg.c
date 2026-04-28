#include "log.h"
#include "log_conf.h"
#include "err_def.h"
#include "msg.h"
#include "emu2000msg.h"

static int gs_emu2000msg_type = 61850;
static MSG gs_emu2000msg_queue = {
    .msg_handle = -1,
    .key = 2014,
};

int emu2000msg_init()
{
    log_info(GLOG, "emu2000msg_init");
    int ret = msg_open(&gs_emu2000msg_queue);
    if (ret < 0) {
        log_error(GLOG, "emu2000msg_init msg_open failed ret=%d", ret);
        return ERR_ACCESS;
    }

    return ERR_OK;
}

int emu2000msg_exit()
{
    log_info(GLOG, "emu2000msg_exit");
    return ERR_OK;
}

int emu2000msg_send(int type, char *data, int len)
{
    if (msg_is_exist(&gs_emu2000msg_queue) < 0) {
        log_warn(GLOG, "emu2000msg_send msg queue not exist");
        return -1;
    }

    if (data == NULL || len <= 0) {
        log_warn(GLOG, "emu2000msg_send invalid data or len");
        return -1;
    }

    MSG_DATA msg_data;
    msg_data.type = type;
    msg_data.data = (char *)data;

    int ret =
        msg_send((MSG *)&gs_emu2000msg_queue, &msg_data, sizeof(long) + len);
    if (ret < 0) {
        log_error(GLOG, "emu2000msg_send failed ret=%d", ret);
        return -1;
    }

    return ret;
}

int emu2000msg_recv(char *data, int len)
{
    if (msg_is_exist(&gs_emu2000msg_queue) < 0) {
        log_warn(GLOG, "emu2000msg_send msg queue not exist");
        return -1;
    }

    if (data == NULL || len <= 0) {
        log_warn(GLOG, "emu2000msg_send invalid data or len");
        return -1;
    }

    MSG_DATA msg_data;
    msg_data.data = (char *)data;
    msg_data.type = gs_emu2000msg_type;

    ssize_t ret = msg_recv((MSG *)&gs_emu2000msg_queue,
                       &msg_data, sizeof(long) + len);
    if (ret < 0) {
        log_error(GLOG, "emu2000msg_send failed ret=%d", ret);
        return -1;
    }

    return ret;
}
