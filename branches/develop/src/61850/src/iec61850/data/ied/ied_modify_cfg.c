#include "err_def.h"

#include "ied_modify_cfg.h"
#include "dev_data.h"
#include "dev_list.h"
#include "iec61850_conf.h"
#include "log_conf.h"

extern IEC61850_CONF *g_pcfg;

static int ied_get_write_config_ld(FILE *f, char *ld_name, char *in_str,
                                   int in_len)
{
    char ld_str[128];
    snprintf(ld_str, sizeof(ld_str), "LD(%s){\n", ld_name);

    fputs(ld_str, f);
    fputs(in_str, f);

    fputs("}\n", f);

    return ERR_OK;
}

static int ied_get_write_config_head(FILE *f, char *ied_name)
{
    char head_str[128];
    snprintf(head_str, sizeof(head_str), "MODEL(%s){\n", ied_name);

    fputs(head_str, f);

    return ERR_OK;
}

static int ied_get_write_config_tail(FILE *f)
{
    fputs("}\n", f);

    return ERR_OK;
}

/**
 *  \brief 获取LD的串
 *  \param "LD name" 如"PXR25"
 *  \param out_str 输出缓冲区
 *  \param out_len 缓冲区长度
 *  \return ERR_OK-成功 其他-失败
 */
static int ied_get_ld_str(const char *file, char *ld_name, char *out_str,
                          int out_len)
{
    if (ld_name == NULL || out_str == NULL || out_len <= 0 || file == NULL) {
        return ERR_PNULL;
    }

    FILE *fp = fopen(file, "r");
    if (fp == NULL) {
        log_error(CFGLOG, "ied_get_ld_str fopen %s failed", file);
        return ERR_IO;
    }

    char line[1024];
    char ld_pattern[32];
    snprintf(ld_pattern, sizeof(ld_pattern), "LD(%s){", ld_name);
    int in_ld_block = 0;
    int brace_count = 0;
    int pos = 0;

    while (fgets(line, sizeof(line), fp)) {
        if (!in_ld_block) {
            if (strstr(line, ld_pattern)) {
                in_ld_block = 1;
                brace_count = 1;
            }
            continue;
        }

        // Count braces to find matching closing brace
        for (char *p = line; *p; p++) {
            if (*p == '{')
                brace_count++;
            if (*p == '}')
                brace_count--;

            if (brace_count == 0) {
                // End of LD block
                out_str[pos] = '\0';
                fclose(fp);
                return ERR_OK;
            }

            if (pos < out_len - 1) {
                out_str[pos++] = *p;
            }
        }
    }

    fclose(fp);
    return ERR_NOTEXIST;
}

int ied_modify_cfg(const char *raw_file, const char *cfg_file)
{
    FILE *dst = NULL;

    DEV_DATA *pied = dev_list_ied_data();
    if (pied == NULL) {
        log_error(PROLOG, "ied_data_init dev_list_ied_data failed");
        goto err1;
    }

    dst = fopen(cfg_file, "w");
    if (dst == NULL) {
        log_error(PROLOG, "model_config_replace fopen dst_file=%s failed",
                  cfg_file);
        goto err1;
    }
    ied_get_write_config_head(dst, pied->devname);
    int i;

    for (i = 0; i < g_pcfg->devlist.num; i++) {
        IEC61850_CFG_DEV *pcdev = &g_pcfg->devlist.devs[i];
        IEC61850_CFG_TEMPLATE *ptemplate =
            iec61850_config_template(g_pcfg, pcdev->template_name);

        if (DEV_TYPE_MATCH_METER(pcdev->type)) {
            char out_str[1024 * 1024] = { 0 };
            char filename[256];
            snprintf(filename, sizeof(filename), "%s/model_%s.cfg", CONFIG_PATH,
                     ptemplate->cfgname);
            log_info(PROLOG, "ied_modify_cfg get ld_str from file %s",
                     filename);
            int ret = ied_get_ld_str(filename, ptemplate->ldname, out_str,
                                     sizeof(out_str));
            if (ret != ERR_OK) {
                log_warn(PROLOG,
                         "ied_get_ld_str %s ldname=%s failed ret=%d(%s)",
                         filename, ptemplate->cfgname, ret, err_str(ret));
                memset(out_str, 0, sizeof(out_str));
                ret = ied_get_ld_str(filename, "LD0", out_str, sizeof(out_str));
                if (ret != ERR_OK) {
                    log_error(PROLOG,
                              "ied_get_ld_str %s ldname=LD0 failed ret=%d(%s)",
                              filename, ret, err_str(ret));
                    goto err1;
                }

                ied_get_write_config_ld(dst, "LD0", out_str, sizeof(out_str));

                continue;
            }

            /* printf("LD Str:\n%s\n", out_str); */
            ied_get_write_config_ld(dst, ptemplate->ldname, out_str,
                                    sizeof(out_str));
        }
    }
    ied_get_write_config_tail(dst);
    fclose(dst);

    return ERR_OK;

err1:
    if (dst) {
        fclose(dst);
    }
    return ERR_NOTEXIST;
}
