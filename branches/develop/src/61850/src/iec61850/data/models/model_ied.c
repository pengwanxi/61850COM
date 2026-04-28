#include <stdio.h>
#include <string.h>

#include <stdlib.h>

#include "iec61850_conf.h"
#include "err_def.h"
#include "list.h"

#include "model_data.h"
#include "model_ied.h"
#include "ied_data.h"
#include "log_conf.h"

extern IEC61850_CONF *g_pcfg;

static void model_idx_free(void *val)
{
    MODEL_IDX *pdata = (MODEL_IDX *)val;
    if (pdata == NULL) {
        return;
    }

    if (pdata->refname != NULL) {
        free(pdata->refname);
        pdata->refname = NULL;
    }

    free(pdata);
}

static void printModelNode(char *name, ModelNode *ModelNode)
{
    if (NULL == ModelNode) {
        return;
    }

    if (ModelNode_getType(ModelNode) == LogicalNodeModelType) {
        char cname[1024];
        snprintf(cname, sizeof(cname), "%s/%s", name, ModelNode_getName(ModelNode));
        /* printf("name=%s\n", cname); */

        printModelNode(name, ModelNode->sibling);
        printModelNode(cname, ModelNode->firstChild);
    }
    else if (ModelNode_getType(ModelNode) == DataObjectModelType) {
        char cname[1024];
        snprintf(cname, sizeof(cname), "%s/%s", name, ModelNode_getName(ModelNode));
        /* printf("name=%s\n", cname); */

        printModelNode(name, ModelNode->sibling);
        printModelNode(cname, ModelNode->firstChild);
    }
    else if (ModelNode_getType(ModelNode) == DataAttributeModelType) {
        char cname[1024];
        snprintf(cname, sizeof(cname), "%s/%s", name, ModelNode_getName(ModelNode));
        /* printf("name=%s\n", cname); */

        printModelNode(name, ModelNode->sibling);
        if (ModelNode->firstChild == NULL) {
            log_debug(GLOG, "name=%s", cname);
        }
        else {
            printModelNode(cname, ModelNode->firstChild);
        }
    }
}

/**
 *  \brief 打印iedModel LD
 *  \param IedModel
 *  \return void
 */
static void printModelLDs(IedModel *iecModel)
{
    int count = IedModel_getLogicalDeviceCount(iecModel);

    log_debug(GLOG, "IEC 61850 IedModel Logical Devices(%d):", count);
    for (int i = 0; i < count; i++) {
        LogicalDevice *ld = IedModel_getDeviceByIndex(iecModel, i);
        log_debug(GLOG, "  LD %d: name=%s", i, ld->name);
        log_debug(GLOG, "  ldname=%s", ld->ldName);
        const char *ModelType =
            ld->modelType == LogicalDeviceModelType ? "LOGICAL_"
                                                      "DEVICE" :
                                                      "UNKNOWN";
        log_debug(GLOG, "  ModelType=%s", ModelType);

        printModelNode(ld->name, ld->parent);
        printModelNode(ld->name, ld->sibling);
        printModelNode(ld->name, ld->firstChild);
    }
}

/**
 *  \brief 打印IedModel *
 *  \param iecModel
 *  \return void
 */
static void printIedModel(IedModel *iecModel)
{
    log_debug(GLOG, "IEC 61850 IedModel:");
    log_debug(GLOG, "name:%s", iecModel->name);

    printModelLDs(iecModel);
}

static void pushModelNode(char *name, ModelNode *ModelNode,
                          list_t *pmodel_idx_list)
{
    if (NULL == ModelNode) {
        return;
    }

    if (ModelNode_getType(ModelNode) == LogicalNodeModelType) {
        char cname[1024];
        snprintf(cname, sizeof(cname), "%s/%s", name, ModelNode_getName(ModelNode));
        /* printf("name=%s\n", cname); */

        pushModelNode(name, ModelNode->sibling, pmodel_idx_list);
        pushModelNode(cname, ModelNode->firstChild, pmodel_idx_list);
    }
    else if (ModelNode_getType(ModelNode) == DataObjectModelType) {
        char cname[1024];
        snprintf(cname, sizeof(cname), "%s/%s", name, ModelNode_getName(ModelNode));
        /* printf("name=%s\n", cname); */

        pushModelNode(name, ModelNode->sibling, pmodel_idx_list);
        pushModelNode(cname, ModelNode->firstChild, pmodel_idx_list);
    }
    else if (ModelNode_getType(ModelNode) == DataAttributeModelType) {
        char cname[1024];
        snprintf(cname, sizeof(cname), "%s/%s", name, ModelNode_getName(ModelNode));
        /* printf("name=%s\n", cname); */

        pushModelNode(name, ModelNode->sibling, pmodel_idx_list);
        if (ModelNode->firstChild == NULL) {
            /* printf("refname=%s\n", cname); */
            DataAttribute *pattr = (DataAttribute *)ModelNode;

            MODEL_IDX *pdata = (MODEL_IDX *)malloc(sizeof(MODEL_IDX));
            if (pdata == NULL) {
                return;
            }
            memset(pdata, 0, sizeof(MODEL_IDX));
            pdata->refname = strdup(cname);
            if (pdata->refname == NULL) {
                free(pdata);
                return;
            }
            pdata->pattr = pattr;

            list_node_t *node = list_node_new(pdata);
            if (node == NULL) {
                free(pdata->refname);
                free(pdata);
                return;
            }
            list_lpush(pmodel_idx_list, node);
        }
        else {
            pushModelNode(cname, ModelNode->firstChild, pmodel_idx_list);
        }
    }
}

/**
 *  \brief 打印iedModel LD
 *  \param IedModel
 *  \return void
 */
static void pushModelLDs(IedModel *iecModel, list_t *pmodel_idx_list)
{
    int count = IedModel_getLogicalDeviceCount(iecModel);

    log_debug(GLOG, "IEC 61850 IedModel Logical Devices(%d):", count);
    for (int i = 0; i < count; i++) {
        LogicalDevice *ld = IedModel_getDeviceByIndex(iecModel, i);
        log_debug(GLOG, "  LD %d: name=%s", i, ld->name);
        log_debug(GLOG, "  ldname=%s", ld->ldName);
        const char *ModelType =
            ld->modelType == LogicalDeviceModelType ? "LOGICAL_"
                                                      "DEVICE" :
                                                      "UNKNOWN";
        log_debug(GLOG, "  ModelType=%s", ModelType);

        pushModelNode(ld->name, ld->parent, pmodel_idx_list);
        pushModelNode(ld->name, ld->sibling, pmodel_idx_list);
        pushModelNode(ld->name, ld->firstChild, pmodel_idx_list);
    }
}


static MODEL_IED_PRIVATE_DATA gs_private_data;

static MODEL_DATA gs_model_data_61850[] = {
    {
        .name = "model_61850",
        .proto = "iec61850",
        .priv_data = &gs_private_data,
    },
};

/**
 *  \brief match IEC61850 模型数据
 *  \param name protocol
 *  \return NULL-失败 其他-成功
 */
static MODEL_DATA *match_model_61850(char *name, char *protocol)
{
    if (name == NULL) {
        return NULL;
    }

    int count = sizeof(gs_model_data_61850) / sizeof(gs_model_data_61850[0]);
    for (int i = 0; i < count; i++) {
        MODEL_DATA *pmodel = &gs_model_data_61850[i];
        if (strcmp(pmodel->name, name) == 0 &&
            strcmp(pmodel->proto, protocol) == 0) {
            return pmodel;
        }
    }

    return NULL;
}

int model_ied_init()
{
    if (g_pcfg == NULL) {
        return ERR_PNULL;
    }

    MODEL_DATA *pmodel = match_model_61850("model_61850", "iec61850");
    if (pmodel == NULL) {
        return ERR_NOTEXIST;
    }

    MODEL_IED_PRIVATE_DATA *ppriv =
        (MODEL_IED_PRIVATE_DATA *)pmodel->priv_data;

    ppriv->model = ied_model();
    if (ppriv->model == NULL) {
        log_error(GLOG,
                  "Error creating IEC 61850 data model from configuration file!");
        return -1;
    }

    /* 分配指针 */
    list_t *pmodel_idx_list = NULL;
    pmodel_idx_list = list_new();
    if (pmodel_idx_list == NULL) {
        return ERR_MEM;
    }
    pmodel_idx_list->free = model_idx_free;
    pushModelLDs(ppriv->model, pmodel_idx_list);

    log_info(GLOG, "DEV DATA LIST(len=%d):", pmodel_idx_list->len);
    PRIVATE_LIST_WHILE_EACH(pmodel_idx_list)
    {
        MODEL_IDX *data = (MODEL_IDX *)node->val;
        if (data) {
            log_debug(GLOG, "refname=%s", data->refname);
            log_debug(GLOG, "name=%s type=%d", data->pattr->name,
                      data->pattr->type);
        }
    }

    int ret = model_data_register(pmodel);
    if (ret != ERR_OK) {
        list_destroy(pmodel_idx_list);
        return ret;
    }

    /* make sure list doesn't leak if caller doesn't consume it */
    if (g_pcfg) {
        g_pcfg->pmodel_idx_list = pmodel_idx_list;
    }
    else {
        list_destroy(pmodel_idx_list);
    }

    return ERR_OK;
}

int model_ied_exit()
{
    if (g_pcfg && g_pcfg->pmodel_idx_list) {
        list_destroy((list_t *)g_pcfg->pmodel_idx_list);
        g_pcfg->pmodel_idx_list = NULL;
    }
    return ERR_OK;
}
