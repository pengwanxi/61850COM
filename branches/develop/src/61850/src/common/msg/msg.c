#include "msg.h"
#include "log.h"
/* #include "core.h" */

#include <stdio.h>
#include <unistd.h>
#include <string.h>

#define MSG_DIR_SEND (0)
#define MSG_DIR_RECV (1)
#define MSG_DIR_ERROR (2)

int msg_create(MSG *pmsg)
{
    pmsg->msg_handle = msgget(pmsg->key, IPC_CREAT | 0666);

    if (-1 != pmsg->msg_handle) {
        log_info(GLOG, "msg_create ok");
        return pmsg->msg_handle;
    }

    log_error(GLOG, "msg_create fail");
    perror("error:");
    pmsg->msg_handle = -1;

    return -2;
}

void msg_destroy(MSG *pmsg)
{
    if (pmsg->msg_handle > 0) {
        int rtn = msgctl(pmsg->msg_handle, IPC_RMID, NULL);
        if (rtn < 0) {
            log_warn(GLOG, "msg_destroy %d fail", pmsg->msg_handle);
            msg_close(pmsg);
            return;
        }

        log_info(GLOG, "msg_destroy %d ok", pmsg->msg_handle);
        msg_close(pmsg);
    }
}

int msg_is_exist(MSG *pmsg)
{
    if (pmsg->msg_handle == -1)
        return -1;

    return 0;
}

int msg_open(MSG *pmsg)
{
    pmsg->msg_handle = msgget(pmsg->key, 0);

    if (-1 != pmsg->msg_handle) {
        log_info(GLOG, "msg_open ok");
        return pmsg->msg_handle;
    }

    log_error(GLOG, "msg_open fail");
    perror("error:");
    pmsg->msg_handle = -1;
    return -2;
}

void msg_close(MSG *pmsg)
{
    pmsg->msg_handle = -1;
}

int msg_send(MSG *pmsg, MSG_DATA *pmsg_data, int len)
{
    if (NULL == pmsg ) {
        log_warn(GLOG, "%s pmsg is NULL ", __FUNCTION__);
        return -1;
    }

    if (NULL == pmsg_data || len <= sizeof(long)) {
        log_warn(GLOG, "%s pmsg_data is NULL ", __FUNCTION__);
        return -1;
    }

    if (-1 == msg_is_exist(pmsg)) {
        log_warn(GLOG, "msg is not created ");
        return -1;
    }

    /* if (msg_dest_valid(pmsg_data) <= 0) { */
    /*     log_warn(GLOG, "msg dest=%lu not valid ", pmsg_data->dest); */
    /*     return -1; */
    /* } */

    int ret = msgsnd(pmsg->msg_handle, pmsg_data,
                     len - sizeof(long), IPC_NOWAIT);

    if (ret < 0) {
        log_warn(MSGLOG, "send ret = %d ", ret);
        perror("error msgsnd:");
    }
    else {
    }

    return ret;
}

ssize_t msg_recv(MSG *pmsg, MSG_DATA *pmsg_data, int max)
{
    ssize_t rtn;
    if (NULL == pmsg ) {
        log_warn(GLOG, "%s pmsg is NULL ", __FUNCTION__);
        return -1;
    }

    if (NULL == pmsg_data || max <= sizeof(long)) {
        log_warn(GLOG, "%s pmsg_data is NULL ", __FUNCTION__);
        return -1;
    }

    if (-1 == msg_is_exist(pmsg)) {
        log_warn(GLOG, "msg is not created ");
        return -1;
    }

    rtn = msgrcv(pmsg->msg_handle, pmsg_data->data, max - sizeof(long),
                 pmsg_data->type, IPC_NOWAIT);
    if (rtn > 0) {
    }

    /* if ( rtn < 0) { */
    /* 	printf("handle=%d type=%lu\n", pmsg->msg_handle, type); */
    /* 	perror("msg_recv %d %d"); */
    /* } */

    return rtn;
}
