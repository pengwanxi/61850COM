#include <stdio.h>
#include <string.h>
#include "mms_server.h"

#include "iec61850_dynamic_model.h"

#include "iec61850_macro.h"
#include "err_def.h"
#include "log.h"
#include "log_conf.h"
#include "ied_data.h"
#include "ied_modify_cfg.h"
#include "ied_refname_table.h"

static IedModel *gs_ied_model = NULL;
static IedServer gs_ied_server = NULL;

int ied_update_float(char *name, float fval)
{
    if (gs_ied_server == NULL || gs_ied_model == NULL) {
        return ERR_PNULL;
    }

    DataAttribute *attr =
        (DataAttribute *)IedModel_getModelNodeByObjectReference(gs_ied_model,
                                                                name);
    if (attr == NULL) {
        log_error(PROLOG,
                  "ied_update_float IedModel_getDataAttributeByName %s failed",
                  name);
        return ERR_NOTEXIST;
    }

    IedServer_updateFloatAttributeValue(gs_ied_server, attr, fval);

    return ERR_OK;
}

int ied_update_ival(char *name, int ival)
{
    if (gs_ied_server == NULL || gs_ied_model == NULL) {
        return ERR_PNULL;
    }

    DataAttribute *attr =
        (DataAttribute *)IedModel_getModelNodeByObjectReference(gs_ied_model,
                                                                name);
    if (attr == NULL) {
        log_error(PROLOG,
                  "ied_update_ival IedModel_getDataAttributeByName %s failed",
                  name);
        return ERR_NOTEXIST;
    }

    IedServer_updateInt32AttributeValue(gs_ied_server, attr, ival);

    return ERR_OK;
}

int ied_update_dbpos(char *name, int dbpos)
{
    if (gs_ied_server == NULL || gs_ied_model == NULL) {
        return ERR_PNULL;
    }

    DataAttribute *attr =
        (DataAttribute *)IedModel_getModelNodeByObjectReference(gs_ied_model,
                                                                name);
    if (attr == NULL) {
        log_error(PROLOG,
                  "ied_update_ival IedModel_getDataAttributeByName %s failed",
                  name);
        return ERR_NOTEXIST;
    }

    Dbpos dbpos_val = (Dbpos)dbpos;
    IedServer_updateDbposValue(gs_ied_server, attr, dbpos_val);

    return ERR_OK;
}

int ied_update_quality(char *name, uint16_t valid)
{
    if (gs_ied_server == NULL || gs_ied_model == NULL) {
        return ERR_PNULL;
    }

    DataAttribute *attr =
        (DataAttribute *)IedModel_getModelNodeByObjectReference(gs_ied_model,
                                                                name);
    if (attr == NULL) {
        log_error(PROLOG,
                  "ied_update_ival IedModel_getDataAttributeByName %s failed",
                  name);
        return ERR_NOTEXIST;
    }

    IedServer_updateQuality(gs_ied_server, attr, valid);

    return ERR_OK;
}

int ied_update_sval(char *name, char *str)
{
    if (gs_ied_server == NULL || gs_ied_model == NULL) {
        return ERR_PNULL;
    }

    DataAttribute *attr =
        (DataAttribute *)IedModel_getModelNodeByObjectReference(gs_ied_model,
                                                                name);
    if (attr == NULL) {
        log_error(PROLOG,
                  "ied_update_ival IedModel_getDataAttributeByName %s failed",
                  name);
        return ERR_NOTEXIST;
    }

    IedServer_updateVisibleStringAttributeValue(gs_ied_server, attr, str);

    return ERR_OK;
}

int ied_update_uival(char *name, unsigned int ival)
{
    if (gs_ied_server == NULL || gs_ied_model == NULL) {
        return ERR_PNULL;
    }

    DataAttribute *attr =
        (DataAttribute *)IedModel_getModelNodeByObjectReference(gs_ied_model,
                                                                name);
    if (attr == NULL) {
        log_error(PROLOG,
                  "ied_update_ival IedModel_getDataAttributeByName %s failed",
                  name);
        return ERR_NOTEXIST;
    }

    IedServer_updateUnsignedAttributeValue(gs_ied_server, attr, ival);

    return ERR_OK;
}

int ied_update_bval(char *name, bool bval)
{
    if (gs_ied_server == NULL || gs_ied_model == NULL) {
        return ERR_PNULL;
    }

    DataAttribute *attr =
        (DataAttribute *)IedModel_getModelNodeByObjectReference(gs_ied_model,
                                                                name);
    if (attr == NULL) {
        log_error(PROLOG,
                  "ied_update_bval IedModel_getDataAttributeByName %s failed",
                  name);
        return ERR_NOTEXIST;
    }

    IedServer_updateBooleanAttributeValue(gs_ied_server, attr, bval);

    return ERR_OK;
}

int ied_update_utctime(char *name, uint64_t ival)
{
    if (gs_ied_server == NULL || gs_ied_model == NULL) {
        return ERR_PNULL;
    }

    DataAttribute *attr =
        (DataAttribute *)IedModel_getModelNodeByObjectReference(gs_ied_model,
                                                                name);
    if (attr == NULL) {
        log_error(PROLOG,
                  "ied_update_bval IedModel_getDataAttributeByName %s failed",
                  name);
        return ERR_NOTEXIST;
    }

    IedServer_updateUTCTimeAttributeValue(gs_ied_server, attr, ival);

    return ERR_OK;
}

int ied_update_all_quality(char *iedname, char *devname, uint16_t value)
{
    IedModel *model = ied_model();
    IED_REFNAME_TABLE_LIST *ptable_list = NULL;

    ptable_list =
        (IED_REFNAME_TABLE_LIST *)malloc(sizeof(IED_REFNAME_TABLE_LIST));
    if (ptable_list == NULL) {
        return ERR_MEM;
    }
    ied_refname_table_init(ptable_list, model);
    int i;
    for (i = 0; i < ptable_list->num; i++) {
        int j;
        for (j = 0; j < ptable_list->tables[i].num; j++) {
            char *name = ptable_list->tables[i].refnames[j].refname;
            if (NULL != strstr(name, ".q")) {
                char refname[1024];
                sprintf(refname, "%s%s/%s", iedname, devname, name);
                /* printf("refname=%s value=%x\n", refname, value); */
                ied_update_quality(refname, value);
            }
        }
    }
    ied_refname_table_exit(ptable_list);

    free(ptable_list);
    ptable_list = NULL;

    return 0;
}

int model_config_replace(const char *src_file, const char *dst_file,
                         const char *search_str, const char *replace_str)
{
    FILE *src = fopen(src_file, "r");
    if (src == NULL) {
        log_error(PROLOG, "model_config_replace fopen src_file=%s failed",
                  src_file);
        return ERR_PNULL;
    }

    FILE *dst = fopen(dst_file, "w");
    if (dst == NULL) {
        log_error(PROLOG, "model_config_replace fopen dst_file=%s failed",
                  dst_file);
        fclose(src);
        return ERR_PNULL;
    }

    char line[1024];
    int replaced = 0;

    while (fgets(line, sizeof(line), src)) {
        char *found = strstr(line, search_str);
        if (found) {
            // 计算替换位置
            int pos = found - line;
            // 写入行开始部分
            fwrite(line, 1, pos, dst);
            // 写入替换字符串
            fputs(replace_str, dst);
            // 写入行剩余部分
            fputs(found + strlen(search_str), dst);
            replaced = 1;
        }
        else {
            fputs(line, dst);
        }
    }

    fclose(src);
    fclose(dst);

    return ERR_OK;
}

IedServer ied_server(void)
{
    return gs_ied_server;
}

IedModel *ied_model(void)
{
    return gs_ied_model;
}
static MmsError fileAccessHandler(void *parameter,
                                  MmsServerConnection connection,
                                  MmsFileServiceType service,
                                  const char *localFilename,
                                  const char *otherFilename)
{
    log_info(PROLOG,
             "fileAccessHandler: service = %i, local-file: %s other-file: %s",
             service, localFilename, otherFilename);
    if (service == MMS_FILE_ACCESS_TYPE_RENAME)
        return MMS_ERROR_FILE_FILE_ACCESS_DENIED;

    log_debug(PROLOG,
              "fileAccessHandler allow access for service %i, local-file: %s "
              "other-file: %s",
              service, localFilename, otherFilename);
    return MMS_ERROR_NONE;
}

int ied_data_init()
{
    int ret;
    /* model_config_replace(MODEL_RAW_FILE, MODEL_CONFIG_FILE, "LD(EM6000)", "LD(EM63)"); */
    log_info(PROLOG, "ied_data_init start");

    log_debug(PROLOG, "ied_data_init modify config file %s from %s",
              MODEL_CONFIG2_FILE, MODEL_RAW_FILE);
    ret = ied_modify_cfg(MODEL_RAW_FILE, MODEL_CONFIG2_FILE);
    if (ret != ERR_OK) {
        log_error(PROLOG, "ied_modify_cfg failed ret=%d", ret);
        return ret;
    }

    ret = ERR_OK;
    IedModel *raw_model = NULL;
    IED_REFNAME_TABLE_LIST *ptable_list = NULL;
    IedModel *model = NULL;
    IedServer server = NULL;
    MmsServer mmsServer = NULL;

    raw_model = ConfigFileParser_createModelFromConfigFileEx(MODEL_RAW_FILE);
    if (raw_model == NULL) {
        log_error(PROLOG, "Error creating IEC 61850 IedModel from raw "
                          "configuration file!");
        ret = ERR_PNULL;
        goto out;
    }

    ptable_list =
        (IED_REFNAME_TABLE_LIST *)malloc(sizeof(IED_REFNAME_TABLE_LIST));
    if (ptable_list == NULL) {
        ret = ERR_MEM;
        goto out;
    }
    ied_refname_table_init(ptable_list, raw_model);
    /* ied_refname_table_print_json(ptable_list); */
    ied_refname_table_write_json(ptable_list);
    ied_refname_table_exit(ptable_list);
    free(ptable_list);
    ptable_list = NULL;

    IedModel_destroy(raw_model);
    raw_model = NULL;

    model = ConfigFileParser_createModelFromConfigFileEx(MODEL_CONFIG2_FILE);
    if (model == NULL) {
        log_error(PROLOG, "Error creating IEC 61850 IedModel from "
                          "configuration file!");
        ret = ERR_PNULL;
        goto out;
    }

    server = IedServer_create(model);
    if (server == NULL) {
        log_error(PROLOG, "Error creating IEC 61850 IedServer instance!");
        ret = ERR_MEM;
        goto out;
    }
    /* printf("server edition=%u\n", server->edition); */
    printf("server %s\n", LibIEC61850_getVersionString());

    /* Set the base path for the MMS file services */
    mmsServer = IedServer_getMmsServer(server);
    log_info(PROLOG, "Setting MMS file service base path to %s",
             MODEL_IED_WAVE_DIR);
    IedServer_setFilestoreBasepath(server, (char *)MODEL_IED_WAVE_DIR);
    /* /\* Set a callback handler to control file accesses *\/ */
    MmsServer_installFileAccessHandler(mmsServer, fileAccessHandler, NULL);

    gs_ied_model = model;
    gs_ied_server = server;

    return ERR_OK;

out:
    printf("error ret=%d\n", ret);
    if (ptable_list) {
        free(ptable_list);
    }
    if (raw_model) {
        IedModel_destroy(raw_model);
    }
    if (server) {
        IedServer_destroy(server);
    }
    if (model) {
        IedModel_destroy(model);
    }
    return ret;
}

int ied_data_exit()
{
    if (gs_ied_server != NULL) {
        IedServer_stop(gs_ied_server);
        IedServer_destroy(gs_ied_server);
        gs_ied_server = NULL;
    }

    if (gs_ied_model != NULL) {
        IedModel_destroy(gs_ied_model);
        gs_ied_model = NULL;
    }

    return ERR_OK;
}
