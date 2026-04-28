/************************************************
 * 配置文件管理模块实现
 * 管理 BusLine.ini、rtdb.conf 和 BusXX.ini
 ************************************************/

#include "../inc/config_manager.h"
#include "../inc/zlog.h"

#define BUSLINE_FILE "/mynand/config/BusLine.ini"
#define RTDB_FILE "/mynand/config/rtdb.conf"
#define MODBUS_DIR "/mynand/config/ModBusMaster"

// zlog日志类别
static zlog_category_t *zlog_category = NULL;

// 模板映射表 - 模块ID和值
typedef struct
{
    const char *name;
    int module_id;
    const char *values;
} template_map_t;

static const template_map_t TEMPLATE_MAP[] = {
    {"EATONPXR20_TCP", 30, "70,60,0,50,0,1"},
    {"EATONPXR20_RTU", 31, "70,60,0,50,0,1"},
    {"EATONPXR25_TCP", 32, "70,60,0,50,0,1"},
    {"EATONPXR25_RTU", 33, "70,60,0,50,0,1"},
    {NULL, 0, NULL}};

// 初始化配置管理器
int config_manager_init(void)
{
    zlog_category = zlog_get_category("config");
    if (!zlog_category)
    {
        zlog_category = zlog_get_category("my_cat");
    }
    zlog_info(zlog_category, "配置管理器初始化");
    return 0;
}

// 获取模板信息
const template_map_t *get_template_info(const char *template_name)
{
    if (!template_name)
        return NULL;

    for (int i = 0; TEMPLATE_MAP[i].name != NULL; i++)
    {
        if (strcmp(template_name, TEMPLATE_MAP[i].name) == 0)
        {
            return &TEMPLATE_MAP[i];
        }
    }
    return NULL;
}

// 解析总线类型（根据port值的前缀）
bus_type_t parse_bus_type(const char *port_value)
{
    if (!port_value)
        return BUS_TYPE_UNKNOWN;

    // 查找冒号位置
    // char *colon = strchr(port_value, ':');
    // size_t prefix_len = colon ? (colon - port_value) : strlen(port_value);

    // 检查前缀
    if (strncasecmp(port_value, "COMRS485", 8) == 0 ||
        strncasecmp(port_value, "RS485", 5) == 0 ||
        strncasecmp(port_value, "rs4", 3) == 0)
    {
        return BUS_TYPE_RS485;
    }

    // 其他视为网口类型
    return BUS_TYPE_ETH0;
}

// 解析串口参数（从port值中提取）
void parse_serial_params(const char *port_value, char *params, size_t size)
{
    if (!port_value || !params || size == 0)
        return;

    params[0] = '\0';
    char *colon = strchr(port_value, ':');
    if (colon)
    {
        strncpy(params, colon + 1, size - 1);
        params[size - 1] = '\0';
    }
}

// 从BusLine.ini加载总线配置
int load_bus_config_from_busline(bus_config_t *config)
{
    if (!config)
        return -1;

    memset(config, 0, sizeof(bus_config_t));

    FILE *fp = fopen(BUSLINE_FILE, "r");
    if (!fp)
    {
        zlog_error(zlog_category, "无法打开BusLine.ini: %s", BUSLINE_FILE);
        return -1;
    }

    char line[512];
    int in_port_section = 0;
    int in_para_section = 0;
    int in_internal_section = 0;

    while (fgets(line, sizeof(line), fp))
    {
        line[strcspn(line, "\n")] = '\0';
        line[strcspn(line, "\r")] = '\0';

        if (line[0] == '\0' || line[0] == '#' || line[0] == ';')
            continue;

        if (line[0] == '[')
        {
            in_port_section = (strstr(line, "[PORT]") != NULL);
            in_para_section = (strstr(line, "[PARA]") != NULL);
            in_internal_section = (strstr(line, "[INTERNAL]") != NULL);
            continue;
        }

        // 解析port字段
        if (in_port_section && strncasecmp(line, "port", 4) == 0)
        {
            int row_idx = 0;
            char port_value[256] = {0};

            if (sscanf(line, "port%02d=%255s", &row_idx, port_value) == 2 ||
                sscanf(line, "port%d=%255[^\n]", &row_idx, port_value) == 2)
            {
                if (row_idx >= 1 && row_idx <= MAX_COMM_ROWS)
                {
                    if (strstr(port_value, "PAUSE"))
                        continue;
                    comm_row_t *row = &config->rows[config->row_count];
                    row->id = row_idx;
                    row->type = parse_bus_type(port_value);
                    char *colon = strchr(port_value, ':');
                    char *underscore = strchr(port_value, '_');
                    if (row->type == BUS_TYPE_RS485 && colon)
                    {
                        size_t name_len = colon - port_value;
                        if (name_len >= sizeof(row->device_name))
                            name_len = sizeof(row->device_name) - 1;
                        strncpy(row->device_name, port_value, name_len);
                        row->device_name[name_len] = '\0';
                        strncpy(row->extra_params, colon + 1, sizeof(row->extra_params) - 1);
                        char *underscore = strchr(port_value, '_');
                        if (underscore && underscore < colon)
                            row->port = atoi(underscore + 1);
                        else
                            row->port = row_idx;
                    }
                    else if (underscore)
                    {
                        size_t name_len = underscore - port_value;
                        if (name_len >= sizeof(row->device_name))
                            name_len = sizeof(row->device_name) - 1;
                        strncpy(row->device_name, port_value, name_len);
                        row->device_name[name_len] = '\0';
                        char *colon = strchr(underscore + 1, ':');
                        if (colon)
                        {
                            *colon = '\0';
                            row->port = atoi(underscore + 1);
                            strncpy(row->ip, colon + 1, sizeof(row->ip) - 1);
                            row->ip[sizeof(row->ip) - 1] = '\0';
                            *colon = ':';
                        }
                        else
                        {
                            row->port = atoi(underscore + 1);
                        }
                    }
                    else
                    {
                        strncpy(row->device_name, port_value, sizeof(row->device_name) - 1);
                        row->port = row_idx;
                    }
                    if (row->type == BUS_TYPE_RS485)
                        strcpy(row->type_name, "RS485");
                    else
                        strcpy(row->type_name, "ETH0、ETH1(Switch Mode)");
                    config->row_count++;
                    zlog_debug(zlog_category, "加载通讯行%d: %s, type=%d", row_idx, row->device_name, row->type);
                }
            }
        }
        if ((in_port_section || in_internal_section) && strncasecmp(line, "internal", 8) == 0)
        {
            int row_idx = 0;
            int internal = 1000;
            if (sscanf(line, "internal%02d=%d", &row_idx, &internal) == 2 ||
                sscanf(line, "internal%d=%d", &row_idx, &internal) == 2)
            {
                if (row_idx >= 1 && row_idx <= MAX_COMM_ROWS)
                {
                    for (int i = 0; i < config->row_count; i++)
                    {
                        if (config->rows[i].id == row_idx)
                        {
                            config->rows[i].internal = internal;
                            break;
                        }
                    }
                }
            }
        }
        if ((in_port_section || in_para_section) && strncasecmp(line, "para", 4) == 0)
        {
            int row_idx = 0;
            char para_value[256] = {0};
            if (sscanf(line, "para%02d=%255s", &row_idx, para_value) == 2 ||
                sscanf(line, "para%d=%255[^\n]", &row_idx, para_value) == 2)
            {
                zlog_debug(zlog_category, "加载para%d=%s", row_idx, para_value);
            }
        }
    }
    fclose(fp);
    zlog_info(zlog_category, "从BusLine.ini加载了%d个通讯行", config->row_count);
    return 0;
}

// 从rtdb.conf加载装置配置
int load_devices_from_rtdb(bus_config_t *config)
{
    if (!config)
        return -1;

    FILE *fp = fopen(RTDB_FILE, "r");
    if (!fp)
    {
        zlog_warn(zlog_category, "无法打开rtdb.conf: %s", RTDB_FILE);
        return -1;
    }

    char line[512];
    // int current_row_idx = 1; // 当前通讯行序号
    // int device_in_row = 0;   // 当前行内的装置计数

    // 首先计算每个通讯行应该有多少装置
    int row_device_count[MAX_COMM_ROWS] = {0};

    while (fgets(line, sizeof(line), fp))
    {
        if (strncasecmp(line, "stn", 3) == 0)
        {
            int stn_idx = 0;
            char device_info[512] = {0};

            if (sscanf(line, "stn%d=%511[^\n]", &stn_idx, device_info) == 2)
            {
                // 解析装置名称，提取通讯行序号
                char name[64] = {0};
                char *comma = strchr(device_info, ',');
                if (comma)
                {
                    *comma = '\0';
                    strcpy(name, device_info);
                    *comma = ',';

                    // 从名称中提取通讯行序号 (如 "1_eth0_1#" 或 "2_rs485_1#")
                    int row_idx = 0;
                    if (sscanf(name, "%d_", &row_idx) == 1)
                    {
                        if (row_idx >= 1 && row_idx <= MAX_COMM_ROWS)
                        {
                            row_device_count[row_idx - 1]++;
                        }
                    }
                }
            }
        }
    }

    // 重新读取文件，分配装置到通讯行
    rewind(fp);
    // int stn_counter = 0;

    while (fgets(line, sizeof(line), fp))
    {
        if (strncasecmp(line, "stn", 3) == 0)
        {
            int stn_idx = 0;
            char device_info[512] = {0};

            if (sscanf(line, "stn%d=%511[^\n]", &stn_idx, device_info) == 2)
            {
                char name[64] = {0};

                char *comma = strchr(device_info, ',');
                if (comma)
                {
                    *comma = '\0';
                    strcpy(name, device_info);

                    // 提取通讯行序号
                    int row_idx = 0;
                    if (sscanf(name, "%d_", &row_idx) == 1)
                    {
                        if (row_idx >= 1 && row_idx <= MAX_COMM_ROWS)
                        {
                            // 找到对应的通讯行
                            comm_row_t *row = NULL;
                            for (int i = 0; i < config->row_count; i++)
                            {
                                if (config->rows[i].id == row_idx)
                                {
                                    row = &config->rows[i];
                                    break;
                                }
                            }

                            if (row && row->device_count < MAX_DEVICES_PER_ROW)
                            {
                                device_t *dev = &row->devices[row->device_count++];
                                dev->id = stn_idx;
                                dev->order = row->device_count;
                                strcpy(dev->name, name);
                            }
                        }
                    }
                }
            }
        }
    }

    fclose(fp);

    int total = 0;
    for (int i = 0; i < config->row_count; i++)
    {
        total += config->rows[i].device_count;
    }
    zlog_info(zlog_category, "从rtdb.conf加载了%d个装置", total);
    return 0;
}

// 从BusXX.ini加载装置详细信息
int load_devices_from_bus_ini(bus_config_t *config)
{
    if (!config)
        return -1;

    zlog_info(zlog_category, "从BusXX.ini加载装置详细信息");

    for (int i = 0; i < config->row_count; i++)
    {
        comm_row_t *row = &config->rows[i];
        row->device_count = 0;
        char filename[256];
        snprintf(filename, sizeof(filename), "%s/Bus%02d.ini", MODBUS_DIR, row->id);
        FILE *fp = fopen(filename, "r");
        if (!fp)
        {
            zlog_warn(zlog_category, "无法打开%s", filename);
            continue;
        }
        char line[512];
        device_t *dev = NULL;
        while (fgets(line, sizeof(line), fp))
        {
            line[strcspn(line, "\n")] = '\0';
            line[strcspn(line, "\r")] = '\0';
            if (line[0] == '\0' || line[0] == '#' || line[0] == ';')
                continue;
            int dev_num = 0;
            if (sscanf(line, "[DEV%03d]", &dev_num) == 1)
            {
                if (row->device_count < MAX_DEVICES_PER_ROW)
                {
                    dev = &row->devices[row->device_count++];
                    memset(dev, 0, sizeof(device_t));
                }
                else
                {
                    dev = NULL;
                }
                continue;
            }
            if (line[0] == '[')
            {
                dev = NULL; // 进入其它段，立即置空
                continue;
            }
            if (dev)
            {
                char key[64] = {0};
                char value[256] = {0};
                if (sscanf(line, "%63[^=]=%255[^\n]", key, value) == 2)
                {
                    char *end = key + strlen(key) - 1;
                    while (end > key && isspace((unsigned char)*end))
                    {
                        *end = '\0';
                        end--;
                    }
                    if (strcmp(key, "addr") == 0)
                    {
                        strncpy(dev->address, value, sizeof(dev->address) - 1);
                        dev->address[sizeof(dev->address) - 1] = '\0';
                        zlog_debug(zlog_category, "  地址: %s", dev->address);
                    }
                    else if (strcmp(key, "name") == 0)
                    {
                        strncpy(dev->name, value, sizeof(dev->name) - 1);
                    }
                    else if (strcmp(key, "template") == 0)
                    {
                        char *dot = strstr(value, ".txt");
                        if (dot)
                            *dot = '\0';
                        dev->template_type = parse_template_type(value);
                        zlog_debug(zlog_category, "  模板: %s -> %d", value, dev->template_type);
                        const template_map_t *tmpl = get_template_info(value);
                        if (tmpl)
                            dev->module = tmpl->module_id;
                    }
                    else if (strcmp(key, "module") == 0)
                    {
                        dev->module = atoi(value);
                        zlog_debug(zlog_category, " module %d", dev->module);
                    }
                    else if (strcmp(key, "serialno") == 0)
                    {
                        dev->id = atoi(value);
                    }
                }
            }
        }
        fclose(fp);
        zlog_info(zlog_category, "从%s加载了%d个装置", filename, row->device_count);
    }
    return 0;
}

// 加载总线配置
int load_bus_config(bus_config_t *config)
{
    if (!config)
        return -1;

    if (load_bus_config_from_busline(config) != 0)
    {
        zlog_warn(zlog_category, "加载BusLine.ini失败");
    }

    /*
     * 如需恢复 rtdb.conf 相关功能：
     * 1. 取消下方注释，重新启用 load_devices_from_rtdb。
     * 2. 保证 load_devices_from_rtdb 函数未被删除。
     *
     * if (load_devices_from_rtdb(config) != 0)
     * {
     *     zlog_warn(zlog_category, "加载rtdb.conf失败");
     * }
     */

    // 只从BusXX.ini加载装置信息，不再读取rtdb.conf
    load_devices_from_bus_ini(config);

    return 0;
}

// 保存BusLine.ini
int save_busline_config(const bus_config_t *config)
{
    if (!config)
        return -1;

    zlog_info(zlog_category, "保存BusLine.ini（仅[PORT]段）");

    // 读取原始文件到内存
    FILE *fp_in = fopen(BUSLINE_FILE, "r");
    if (!fp_in)
    {
        zlog_error(zlog_category, "无法打开原始BusLine.ini");
        return -1;
    }
    char *lines[8192];
    int line_count = 0;
    char buf[512];
    while (fgets(buf, sizeof(buf), fp_in) && line_count < 8192)
    {
        lines[line_count] = strdup(buf);
        line_count++;
    }
    fclose(fp_in);

    // 打开临时文件写入
    char tmp_file[256];
    snprintf(tmp_file, sizeof(tmp_file), "%s.tmp", BUSLINE_FILE);
    FILE *fp = fopen(tmp_file, "w");
    if (!fp)
    {
        zlog_error(zlog_category, "无法创建临时文件");
        for (int i = 0; i < line_count; i++)
            free(lines[i]);
        return -1;
    }

    int in_port_section = 0;
    for (int i = 0; i < line_count; i++)
    {
        // 检查段头
        if (lines[i][0] == '[')
        {
            if (strncasecmp(lines[i], "[PORT]", 6) == 0)
            {
                // 写入新的 [PORT] 段头
                fputs("[PORT]\n", fp);
                // 写入 config->rows
                int max_row_id = 0;
                for (int k = 0; k < config->row_count; k++)
                {
                    if (config->rows[k].id > max_row_id)
                        max_row_id = config->rows[k].id;
                }
                for (int line_num = 1; line_num <= max_row_id; line_num++)
                {
                    const comm_row_t *row = NULL;
                    for (int k = 0; k < config->row_count; k++)
                    {
                        if (config->rows[k].id == line_num)
                        {
                            row = &config->rows[k];
                            break;
                        }
                    }
                    if (row)
                    {
                        if (row->type == BUS_TYPE_RS485)
                        {
                            fprintf(fp, "port%02d=COMRS485_%d:%s\n", line_num, row->port,
                                    row->extra_params[0] ? row->extra_params : "9600,e,8,1");
                            fprintf(fp, "para%02d=./lib/libModBusMaster.so\n", line_num);
                            int internal = (row->internal >= 10) ? row->internal : 1000;
                            fprintf(fp, "internal%02d=%d\n\n", line_num, internal);
                        }
                        else
                        {
                            const char *ip = row->ip[0] ? row->ip : "168.128.1.1";
                            fprintf(fp, "port%02d=TCPCLIENT_%d:%s\n", line_num, row->port, ip);
                            fprintf(fp, "para%02d=./lib/libModBusMaster.so\n", line_num);
                            int internal = (row->internal >= 10) ? row->internal : 1000;
                            fprintf(fp, "internal%02d=%d\n\n", line_num, internal);
                        }
                    }
                    else
                    {
                        fprintf(fp, "port%02d=PAUSE\n\n", line_num);
                    }
                }
                // 补齐剩余PAUSE
                int start_pause = (config->row_count == 0) ? 2 : config->row_count + 1;
                for (int k = start_pause; k <= MAX_COMM_ROWS; k++)
                {
                    fprintf(fp, "port%02d=PAUSE\n\n", k);
                }
                in_port_section = 1;
                continue;
            }
            else
            {
                in_port_section = 0;
            }
        }
        if (in_port_section)
        {
            // 跳过原 [PORT] 段内容
            if (lines[i][0] == '[')
            {
                in_port_section = 0;
                fputs(lines[i], fp);
            }
            continue;
        }
        fputs(lines[i], fp);
    }
    for (int i = 0; i < line_count; i++)
        free(lines[i]);

    fclose(fp);
    if (rename(tmp_file, BUSLINE_FILE) != 0)
    {
        zlog_error(zlog_category, "无法替换BusLine.ini");
        unlink(tmp_file);
        return -1;
    }
    else
    {
        chmod(BUSLINE_FILE, 0666);
    }
    system("sync");

    zlog_info(zlog_category, "BusLine.ini保存成功");
    return 0;
}

// 保存rtdb.conf
int save_rtdb_config(const bus_config_t *config)
{
    if (!config)
        return -1;

    zlog_info(zlog_category, "保存rtdb.conf");

    char tmp_file[256];
    snprintf(tmp_file, sizeof(tmp_file), "%s.tmp", RTDB_FILE);
    FILE *fp = fopen(tmp_file, "w");
    if (!fp)
    {
        zlog_error(zlog_category, "无法创建临时文件");
        return -1;
    }

    // 计算总装置数
    int total = 0;
    for (int i = 0; i < config->row_count; i++)
    {
        total += config->rows[i].device_count;
    }

    fprintf(fp, "extend_size=0\n");
    fprintf(fp, "station_sum=%d\n\n", total);

    // 按通讯行顺序写入装置
    int stn_idx = 1;
    for (int i = 0; i < config->row_count; i++)
    {
        const comm_row_t *row = &config->rows[i];
        for (int j = 0; j < row->device_count; j++)
        {
            const device_t *dev = &row->devices[j];
            const template_map_t *tmpl = get_template_info(
                template_type_to_string(dev->template_type));

            if (tmpl)
            {
                fprintf(fp, "stn%d=%s,%s\n", stn_idx++, dev->name, tmpl->values);
            }
        }
    }

    fclose(fp);

    if (rename(tmp_file, RTDB_FILE) != 0)
    {
        zlog_error(zlog_category, "无法替换rtdb.conf");
        unlink(tmp_file);
        return -1;
    }
    else
    {
        chmod(RTDB_FILE, 0666); // 设置文件权限
    }
    system("sync");

    zlog_info(zlog_category, "rtdb.conf保存成功，共%d个装置", total);
    return 0;
}

// 清理不再使用的BusXX.ini文件
void cleanup_bus_ini_files(const bus_config_t *config)
{
    if (!config)
        return;

    zlog_info(zlog_category, "清理不再使用的BusXX.ini文件, 当前配置有%d个通讯行", config->row_count);

    // 遍历ModbusMaster目录下的所有BusXX.ini文件
    char cmd[512];
    snprintf(cmd, sizeof(cmd), "ls -1 %s/Bus*.ini 2>/dev/null", MODBUS_DIR);

    FILE *fp = popen(cmd, "r");
    if (!fp)
    {
        zlog_warn(zlog_category, "无法列出BusXX.ini文件, 目录可能不存在");
        return;
    }

    char filename[512];
    int deleted_count = 0;
    int checked_count = 0;

    while (fgets(filename, sizeof(filename), fp))
    {
        // 移除换行符
        filename[strcspn(filename, "\n")] = '\0';
        filename[strcspn(filename, "\r")] = '\0';

        if (strlen(filename) == 0)
            continue;

        checked_count++;
        zlog_debug(zlog_category, "检查文件: %s", filename);

        // 提取文件名中的编号 (Bus01.ini -> 1)
        int file_row_id = 0;
        char *basename = strrchr(filename, '/');
        if (basename)
        {
            basename++; // 跳过 '/'
        }
        else
        {
            basename = filename;
        }

        zlog_debug(zlog_category, "文件名: %s", basename);

        if (sscanf(basename, "Bus%d.ini", &file_row_id) == 1)
        {
            zlog_debug(zlog_category, "文件对应通讯行编号: %d", file_row_id);

            // 检查该编号是否在当前配置中且有效
            int found = 0;
            for (int i = 0; i < config->row_count; i++)
            {
                zlog_debug(zlog_category, "  检查配置中的通讯行: id=%d, device_count=%d",
                           config->rows[i].id, config->rows[i].device_count);
                if (config->rows[i].id == file_row_id && config->rows[i].device_count > 0)
                {
                    found = 1;
                    zlog_debug(zlog_category, "  -> 找到匹配的通讯行");
                    break;
                }
            }

            if (!found)
            {
                zlog_info(zlog_category, "文件 %s (通讯行%d) 不再使用,准备删除", filename, file_row_id);
                // 删除不再使用的文件
                if (unlink(filename) == 0)
                {
                    zlog_info(zlog_category, "成功删除文件: %s", filename);
                    deleted_count++;
                }
                else
                {
                    zlog_error(zlog_category, "无法删除文件 %s: %s", filename, strerror(errno));
                }
            }
            else
            {
                zlog_debug(zlog_category, "文件 %s 正在使用中,保留", filename);
            }
        }
        else
        {
            zlog_warn(zlog_category, "无法从文件名解析通讯行编号: %s", basename);
        }
    }

    pclose(fp);
    zlog_info(zlog_category, "清理完成, 检查了%d个文件, 删除了%d个文件", checked_count, deleted_count);
}

// 生成BusXX.ini文件
int generate_bus_ini_files(const bus_config_t *config)
{
    if (!config)
        return -1;

    // 创建目录
    char cmd[256];
    snprintf(cmd, sizeof(cmd), "mkdir -p %s", MODBUS_DIR);
    system(cmd);

    // 先清理不再使用的文件
    cleanup_bus_ini_files(config);

    int file_count = 0;
    int global_device_idx = 0; // 全局装置序号（从0开始，对应rtdb.conf中的stn序号-1）

    for (int i = 0; i < config->row_count; i++)
    {
        const comm_row_t *row = &config->rows[i];

        // 如果通讯行没有装置，跳过不生成文件
        if (row->device_count == 0)
        {
            continue;
        }

        // 使用实际的port编号生成文件名（不是数组索引）
        char filename[256];
        snprintf(filename, sizeof(filename), "%s/Bus%02d.ini", MODBUS_DIR, row->id);

        FILE *fp = fopen(filename, "w");
        if (!fp)
        {
            zlog_error(zlog_category, "无法创建%s", filename);
            continue;
        }

        fprintf(fp, "[DEVNUM]\n");
        fprintf(fp, "NUM=%d\n\n", row->device_count);

        for (int j = 0; j < row->device_count; j++)
        {
            const device_t *dev = &row->devices[j];
            const template_map_t *tmpl = get_template_info(
                template_type_to_string(dev->template_type));

            fprintf(fp, "[DEV%03d]\n", j + 1);
            fprintf(fp, "module=%d\n", tmpl ? tmpl->module_id : 0);
            fprintf(fp, "serialno=%d\n", global_device_idx++); // 全局序号，从0开始
            fprintf(fp, "addr=%s\n", dev->address);
            fprintf(fp, "name=%s\n", dev->name);
            fprintf(fp, "sysid=\n");
            fprintf(fp, "ipwave=\n");
            fprintf(fp, "type=\n");
            fprintf(fp, "port=\n");
            fprintf(fp, "user=\n");
            fprintf(fp, "pwd=\n");
            fprintf(fp, "template=%s.txt\n", template_type_to_string(dev->template_type));
            fprintf(fp, "ModuleName=%s\n\n", template_type_to_string(dev->template_type));
        }

        fclose(fp);
        file_count++;
        zlog_debug(zlog_category, "生成%s，包含%d个装置", filename, row->device_count);
    }

    system("sync");
    zlog_info(zlog_category, "生成了%d个BusXX.ini文件", file_count);
    return 0;
}

// 保存总线配置
int save_bus_config(const bus_config_t *config)
{
    if (!config)
        return -1;

    if (save_busline_config(config) != 0)
    {
        zlog_error(zlog_category, "保存BusLine.ini失败");
        return -1;
    }

    /*
     * 如需恢复 rtdb.conf 相关功能：
     * 1. 取消下方注释，重新启用 save_rtdb_config。
     * 2. 保证 save_rtdb_config 函数未被删除。
     *
     * if (save_rtdb_config(config) != 0)
     * {
     *     zlog_error(zlog_category, "保存rtdb.conf失败");
     *     return -1;
     * }
     */

    if (generate_bus_ini_files(config) != 0)
    {
        zlog_warn(zlog_category, "生成BusXX.ini文件失败");
    }

    return 0;
}

// 解析模板类型
template_type_t parse_template_type(const char *template_str)
{
    if (!template_str)
        return TEMPLATE_UNKNOWN;

    if (strcmp(template_str, "EATONPXR20_TCP") == 0)
        return TEMPLATE_EATONPXR20_TCP;
    if (strcmp(template_str, "EATONPXR20_RTU") == 0)
        return TEMPLATE_EATONPXR20_RTU;
    if (strcmp(template_str, "EATONPXR25_TCP") == 0)
        return TEMPLATE_EATONPXR25_TCP;
    if (strcmp(template_str, "EATONPXR25_RTU") == 0)
        return TEMPLATE_EATONPXR25_RTU;

    return TEMPLATE_UNKNOWN;
}

// 模板类型转字符串
const char *template_type_to_string(template_type_t tmpl)
{
    switch (tmpl)
    {
    case TEMPLATE_EATONPXR20_TCP:
        return "EATONPXR20_TCP";
    case TEMPLATE_EATONPXR20_RTU:
        return "EATONPXR20_RTU";
    case TEMPLATE_EATONPXR25_TCP:
        return "EATONPXR25_TCP";
    case TEMPLATE_EATONPXR25_RTU:
        return "EATONPXR25_RTU";
    default:
        return "";
    }
}

// 协议类型（保持兼容）
const char *protocol_type_to_string(protocol_type_t protocol)
{
    return "modbusmaster";
}

const char *protocol_type_to_display_name(protocol_type_t protocol)
{
    return "Modbus Master";
}

// 解析协议类型
protocol_type_t parse_protocol_type(const char *protocol_str)
{
    if (!protocol_str)
        return PROTOCOL_MODBUS_MASTER;

    if (strcmp(protocol_str, "modbusmaster") == 0 ||
        strcmp(protocol_str, "Modbus Master") == 0)
    {
        return PROTOCOL_MODBUS_MASTER;
    }

    return PROTOCOL_MODBUS_MASTER;
}

// 根据模板类型获取模块ID
int template_to_module(template_type_t tmpl)
{
    switch (tmpl)
    {
    case TEMPLATE_EATONPXR20_TCP:
    case TEMPLATE_EATONPXR20_RTU:
        return MODULE_PXR_20;
    case TEMPLATE_EATONPXR25_TCP:
    case TEMPLATE_EATONPXR25_RTU:
        return MODULE_PXR_25;
    default:
        return 0;
    }
}

const char *bus_type_to_string(bus_type_t type)
{
    switch (type)
    {
    case BUS_TYPE_ETH0:
        return "eth0";
    case BUS_TYPE_RS485:
        return "rs485";
    default:
        return "";
    }
}

const char *bus_type_to_display_name(bus_type_t type)
{
    switch (type)
    {
    case BUS_TYPE_ETH0:
        return "ETH0、ETH1(Switch Mode)";
    case BUS_TYPE_RS485:
        return "RS485";
    default:
        return "Unknown";
    }
}
