#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "err_def.h"
#include "iec61850_conf.h"
#include "log_conf.h"

#include "ied_refname_table.h"
#include "file_operation.h"
#include "cJSON.h"

/**
 *  \brief 解析ied_mode
 *  \param ptable_list
 *  \param ied_model
 *  \return ERR_OK-成功 其他-失败
 */
static int ied_refname_table_make_name(char *dst, size_t dst_len,
                                       const char *prefix,
                                       const char *node_name)
{
    if (dst == NULL || dst_len == 0 || node_name == NULL) {
        return ERR_PNULL;
    }

    int n;
    if (prefix == NULL || prefix[0] == '\0') {
        n = snprintf(dst, dst_len, "%s", node_name);
    }
    else {
        n = snprintf(dst, dst_len, "%s.%s", prefix, node_name);
    }

    if (n < 0 || (size_t)n >= dst_len) {
        dst[dst_len - 1] = '\0';
        return ERR_OVERFLOW;
    }

    return ERR_OK;
}

static int ied_refname_table_parse_model_node(IED_REFNAME_TABLE *ptable,
                                              const char *name,
                                              ModelNode *ModelNode)
{
    if (ptable == NULL) {
        return ERR_PNULL;
    }

    if (NULL == ModelNode) {
        return ERR_OK;
    }

    if (ModelNode_getType(ModelNode) == LogicalNodeModelType) {
        char cname[1024];
        int ret = ied_refname_table_make_name(cname, sizeof(cname), name,
                                              ModelNode_getName(ModelNode));
        if (ret != ERR_OK) {
            return ret;
        }
        /* printf("name=%s\n", cname); */

        int rret = ied_refname_table_parse_model_node(ptable, name,
                                                      ModelNode->sibling);
        if (rret != ERR_OK) {
            return rret;
        }
        return ied_refname_table_parse_model_node(ptable, cname,
                                                  ModelNode->firstChild);
    }
    else if (ModelNode_getType(ModelNode) == DataObjectModelType) {
        char cname[1024];
        int ret = ied_refname_table_make_name(cname, sizeof(cname), name,
                                              ModelNode_getName(ModelNode));
        if (ret != ERR_OK) {
            return ret;
        }
        /* printf("name=%s\n", cname); */

        int rret = ied_refname_table_parse_model_node(ptable, name,
                                                      ModelNode->sibling);
        if (rret != ERR_OK) {
            return rret;
        }
        return ied_refname_table_parse_model_node(ptable, cname,
                                                  ModelNode->firstChild);
    }
    else if (ModelNode_getType(ModelNode) == DataAttributeModelType) {
        char cname[1024];
        int ret = ied_refname_table_make_name(cname, sizeof(cname), name,
                                              ModelNode_getName(ModelNode));
        if (ret != ERR_OK) {
            return ret;
        }
        /* printf("name=%s\n", cname); */

        int rret = ied_refname_table_parse_model_node(ptable, name,
                                                      ModelNode->sibling);
        if (rret != ERR_OK) {
            return rret;
        }
        if (ModelNode->firstChild == NULL) {
            if (ptable->num >= IED_REFNAME_MAX) {
                log_error(GLOG, "IED_REFNAME_TABLE %s num=%d exceed max %d",
                          ptable->name, ptable->num, IED_REFNAME_MAX);
                return ERR_OVERFLOW;
            }

            char *refname = strdup(cname);
            if (refname == NULL) {
                return ERR_MEM;
            }
            ptable->refnames[ptable->num++].refname = refname;
        }
        else {
            return ied_refname_table_parse_model_node(ptable, cname,
                                                      ModelNode->firstChild);
        }
    }
    else {
        log_notice(GLOG, "IED_REFNAME_TABLE unknown ModelNode type=%d",
                  ModelNode_getType(ModelNode));
    }

    return ERR_OK;
}

/**
 *  \brief 解析ied_mode
 *  \param ptable_list
 *  \param ied_model
 *  \return ERR_OK-成功 其他-失败
 */
static int ied_refname_table_parse(IED_REFNAME_TABLE_LIST *ptable_list,
                                   IedModel *ied_model)
{
    if (ptable_list == NULL || ied_model == NULL) {
        return ERR_PNULL;
    }

    int count = IedModel_getLogicalDeviceCount(ied_model);
    if (count <= 0) {
        return ERR_NOTEXIST;
    }
    if (count > IED_REFNAME_TABLE_MAX) {
        count = IED_REFNAME_TABLE_MAX;
    }

    log_debug(GLOG, "IEC 61850 IedModel Logical Devices(%d):", count);
    for (int i = 0; i < count; i++) {
        LogicalDevice *ld = IedModel_getDeviceByIndex(ied_model, i);
        log_debug(GLOG, "  LD %d: name=%s", i, ld->name);
        log_debug(GLOG, "  ldname=%s", ld->ldName);
        const char *ModelType = ld->modelType == LogicalDeviceModelType ?
                                    "LOGICAL_"
                                    "DEVICE" :
                                    "UNKNOWN";
        log_debug(GLOG, "  ModelType=%s", ModelType);
        IED_REFNAME_TABLE *ptable = &ptable_list->tables[i];

        ptable->name = strdup(ld->name);
        if (ptable->name == NULL) {
            ptable_list->num = i + 1;
            ied_refname_table_exit(ptable_list);
            return ERR_MEM;
        }

        ptable_list->num = i + 1;

        int ret;
        ret = ied_refname_table_parse_model_node(ptable, NULL, ld->parent);
        if (ret != ERR_OK) {
            ied_refname_table_exit(ptable_list);
            return ret;
        }
        ret = ied_refname_table_parse_model_node(ptable, NULL, ld->sibling);
        if (ret != ERR_OK) {
            ied_refname_table_exit(ptable_list);
            return ret;
        }
        ret = ied_refname_table_parse_model_node(ptable, NULL, ld->firstChild);
        if (ret != ERR_OK) {
            ied_refname_table_exit(ptable_list);
            return ret;
        }
    }
    ptable_list->num = count;

    return ERR_OK;
}

/**
 *  \brief 打印table_list
 *  \param ptable_list
 *  \return void
 */
void ied_refname_table_print(IED_REFNAME_TABLE_LIST *ptable_list)
{
    if (ptable_list == NULL) {
        return;
    }

    for (int i = 0; i < ptable_list->num; i++) {
        IED_REFNAME_TABLE *ptable = &ptable_list->tables[i];
        log_info(GLOG, "IED_REFNAME_TABLE %d: num=%d", i, ptable->num);
        for (int j = ptable->num - 1; j >= 0; j--) {
            if (ptable->refnames[j].refname == NULL) {
                continue;
            }

            log_info(GLOG, "  refname[%d]=%s", j, ptable->refnames[j].refname);
        }
    }
}

/**
 *  \brief 打印为json 文件
 *  \param ptable_list
 *  \return void
 */
void ied_refname_table_print_json(IED_REFNAME_TABLE_LIST *ptable_list)
{
    if (ptable_list == NULL) {
        return;
    }

    printf("{\n");
    for (int i = 0; i < ptable_list->num; i++) {
        IED_REFNAME_TABLE *ptable = &ptable_list->tables[i];
        printf("  \"%s\": [\n", ptable->name);
        for (int j = ptable->num - 1; j >= 0; j--) {
            if (ptable->refnames[j].refname == NULL) {
                continue;
            }

            printf("    \"%s\"", ptable->refnames[j].refname);
            if (j < ptable->num - 1) {
                printf(",");
            }
            printf("\n");
        }
        printf("  ]");
        if (i < ptable_list->num - 1) {
            printf(",");
        }
        printf("\n");
    }
    printf("}\n");
}

void ied_refname_table_write_json(IED_REFNAME_TABLE_LIST *ptable_list)
{
    if (ptable_list == NULL) {
        return;
    }

    if (file_operation_exist(MODEL_REFNAME_TABLE_FILE)) {
        log_info(GLOG, "ied_refname_table_print_json %s exist, remove it first",
                 MODEL_REFNAME_TABLE_FILE);
        remove(MODEL_REFNAME_TABLE_FILE);
    }

    cJSON *root = cJSON_CreateObject();
    if (root == NULL) {
        return;
    }

    for (int i = 0; i < ptable_list->num; i++) {
        IED_REFNAME_TABLE *ptable = &ptable_list->tables[i];
        cJSON *refname_array = cJSON_CreateArray();
        if (refname_array == NULL) {
            cJSON_Delete(root);
            return;
        }

        for (int j = ptable->num - 1; j >= 0; j--) {
            if (ptable->refnames[j].refname != NULL) {
                cJSON_AddItemToArray(
                    refname_array,
                    cJSON_CreateString(ptable->refnames[j].refname));
            }
        }
        cJSON_AddItemToObject(root, ptable->name, refname_array);
    }

    char *json_str = cJSON_Print(root);
    if (json_str != NULL) {
        file_operation_write(MODEL_REFNAME_TABLE_FILE,
                             (unsigned char *)json_str, strlen(json_str));
        free(json_str);
    }

    cJSON_Delete(root);

    system("chmod 777 " MODEL_REFNAME_TABLE_FILE);
}

int ied_refname_table_init(IED_REFNAME_TABLE_LIST *ptable_list,
                           IedModel *ied_model)
{
    log_info(GLOG, "ied_refname_table_init start");
    if (NULL == ptable_list || NULL == ied_model) {
        log_error(GLOG, "ied_refname_table_init ptable_list or ied_model is "
                        "NULL");
        return ERR_PNULL;
    }
    memset(ptable_list, 0, sizeof(IED_REFNAME_TABLE_LIST));

    return ied_refname_table_parse(ptable_list, ied_model);
}
int ied_refname_table_exit(IED_REFNAME_TABLE_LIST *ptable_list)
{
    if (ptable_list == NULL) {
        return ERR_PNULL;
    }

    for (int i = 0; i < ptable_list->num; i++) {
        IED_REFNAME_TABLE *ptable = &ptable_list->tables[i];
        for (int j = 0; j < ptable->num; j++) {
            if (ptable->refnames[j].refname != NULL) {
                free(ptable->refnames[j].refname);
                ptable->refnames[j].refname = NULL;
            }
        }
        ptable->num = 0;
        if (ptable->name != NULL) {
            free(ptable->name);
            ptable->name = NULL;
        }
    }
    ptable_list->num = 0;

    return ERR_OK;
}
