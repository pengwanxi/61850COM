#include <string.h>

#include "err_def.h"
#include "dev_data.h"
#include "dev_val_set.h"

#include "log_conf.h"

static DEV_DATA gs_dev_data[DEV_DATA_MAX];
int gs_dev_data_count = 0;

/**
 *  \brief match 装置数据
 *  \param char *name
 *  \return NULL-失败 其他-成功
 */
DEV_DATA *match_dev_data(char *name)
{
    int i;
    int count =
        gs_dev_data_count <= DEV_DATA_MAX ? gs_dev_data_count : DEV_DATA_MAX;
    for (i = 0; i < count; i++) {
        log_debug(DEVLOG, "match_dev_data name=%s vs %s", name,
                  gs_dev_data[i].name);
        if (strcmp(gs_dev_data[i].name, name) == 0) {
            return &gs_dev_data[i];
        }
    }

    return NULL;
}

int dev_data_register(DEV_DATA *pdev)
{
    if (pdev == NULL || pdev->name == NULL) {
        return ERR_PNULL;
    }

    DEV_DATA *pdev_data = match_dev_data(pdev->name);
    if (pdev_data != NULL) {
        return ERR_OK;
    }

    if (gs_dev_data_count >= DEV_DATA_MAX) {
        return ERR_O_RANGE;
    }

    memcpy(&gs_dev_data[gs_dev_data_count], pdev, sizeof(DEV_DATA));

    gs_dev_data_count += 1;

    log_info(DEVLOG, "dev_data_register name=%s registered count=%d",
             pdev->name, gs_dev_data_count);

    return ERR_OK;
}

int dev_data_unregister(DEV_DATA *pdev)
{
    if (pdev == NULL || pdev->name == NULL) {
        return ERR_PNULL;
    }

    DEV_DATA *pdev_data = match_dev_data(pdev->name);
    if (pdev_data != NULL) {
        return ERR_OK;
    }

    int i;
    int move = -1;
    for (i = 0; i < gs_dev_data_count; i++) {
        DEV_DATA *pdev_data = &gs_dev_data[i];
        if (strcmp(pdev_data->name, pdev->name) == 0) {
            move = i;
            break;
        }

        if (move >= 0) {
            memcpy(&gs_dev_data[i - 1], &gs_dev_data[i], sizeof(DEV_DATA));
        }
    }

    if (gs_dev_data_count > 1) {
        gs_dev_data_count -= 1;
    }

    log_info(DEVLOG, "dev_data_unregister name=%s registered count=%d",
             pdev->name, gs_dev_data_count);

    return ERR_OK;
}

int dev_data_init(DEV_DATA *pdev, char *name)
{
    if (pdev == NULL || name == NULL) {
        return ERR_PNULL;
    }

    DEV_DATA *pdev_data = match_dev_data(name);
    if (pdev_data == NULL) {
        log_error(DEVLOG, "dev_data_init name=%s not exist", name);
        return ERR_NOTEXIST;
    }

    memcpy(pdev, pdev_data, sizeof(DEV_DATA));

    return ERR_OK;
}

int dev_data_exit(DEV_DATA *pdev)
{
    if (pdev == NULL) {
        return ERR_PNULL;
    }

    free(pdev->devname);
    free(pdev->port);
    free(pdev->addr);
    memset(pdev, 0, sizeof(DEV_DATA));

    return ERR_OK;
}
