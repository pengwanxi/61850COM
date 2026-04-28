#include <string.h>

#include "err_def.h"
#include "iec61850_macro.h"
#include "model_data.h"

static MODEL_DATA gs_model_data[MODEL_DATA_MAX];
static int gs_model_data_count = 0;

/**
 *  \brief match 模板数据
 *  \param char *name
 *  \return NULL-失败 其他-成功
 */
MODEL_DATA *match_model_data(char *name)
{
    int i;
    int count =
        gs_model_data_count <= MODEL_DATA_MAX ? gs_model_data_count : MODEL_DATA_MAX;
    for (i = 0; i < count; i++) {
        if (strcmp(gs_model_data[i].name, name) == 0) {
            return &gs_model_data[i];
        }
    }
    return NULL;
}

void *model_private_data(char *name)
{
    MODEL_DATA *pmodel = match_model_data(name);
    if (pmodel == NULL) {
        return NULL;
    }

    return pmodel->priv_data;
}

int model_data_register(MODEL_DATA *pmodel)
{
    if (pmodel == NULL || pmodel->name == NULL) {
        return ERR_PNULL;
    }

    MODEL_DATA *pmodel_data = match_model_data(pmodel->name);
    if (pmodel_data != NULL) {
        return ERR_OK;
    }

    if (gs_model_data_count >= MODEL_DATA_MAX) {
        return ERR_O_RANGE;
    }

    memcpy(&gs_model_data[gs_model_data_count], pmodel, sizeof(MODEL_DATA));
    gs_model_data_count += 1;

    return ERR_OK;
}

int model_data_init(MODEL_DATA *pmodel, char *name)
{
    if (pmodel == NULL || name == NULL) {
        return ERR_PNULL;
    }

    MODEL_DATA *pmodel_data = match_model_data(name);
    if (pmodel_data == NULL) {
        return ERR_NOTEXIST;
    }

    memcpy(pmodel, pmodel_data, sizeof(MODEL_DATA));

    return ERR_OK;
}

int model_data_exit(MODEL_DATA *pmodel)
{
    if (pmodel == NULL) {
        return ERR_PNULL;
    }

    memset(pmodel, 0, sizeof(MODEL_DATA));

    return ERR_OK;
}
