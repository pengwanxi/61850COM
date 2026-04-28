/************************************************
 * EMU2000 Web CGI 程序 - 使用 cJSON
 * 编译: gcc -o web_EMU2000.fcgi web_EMU2000.c config_manager.c -lfcgi -lzlog -lcjson -lpthread
 ************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <stdint.h>
#include <unistd.h>
#include <fcgi_stdio.h>
#include <sys/stat.h>
#include <sys/statvfs.h>
#include <ctype.h>
#include <crypt.h>
#include <time.h>
#include "../inc/zlog.h"
#include "../inc/config_manager.h"
#include "../inc/cJSON.h"

#define ZLOG_CONF_FILE "/mynand/web/config/zlog.conf"
#define PASSWD_FILE "/mynand/web/config/passwd"
#define FALLBACK_PASSWD_FILE "config/passwd"
#define CONFIG_FILE "/mynand/web/config/config.cfg"
#define BUSLINE_FILE "/mynand/config/BusLine.ini"
#define COMTRADE_FILE "/mynand/config/comtrade.conf"
#define COM_STATUS_FILE "/mynand/data/link.status"
#define MAX_POST_SIZE 10485760 /* 10MB */
#define MAX_LINE_LENGTH 256
// 上传升级文件方面的定义
#define FIRMWARE_DIR "/mynand/firmware"
#define UPGRADE_STATUS_FILE "/mynand/firmware/upgrade_status.json"
#define LOG_PATH_61850 "/mynand/log" /*61850日志配置路径*/
#define BUFFER_SIZE 4096
#define MAX_CHUNK_SIZE (1024 * 1024)
#define MAX_FILENAME 128

/* zlog类别 */
static zlog_category_t *zlog_cat = NULL;

/* 简化日志宏 */
#define LOG_DEBUG(fmt, ...) zlog_debug(zlog_cat, fmt, ##__VA_ARGS__)
#define LOG_INFO(fmt, ...) zlog_info(zlog_cat, fmt, ##__VA_ARGS__)
#define LOG_WARN(fmt, ...) zlog_warn(zlog_cat, fmt, ##__VA_ARGS__)
#define LOG_ERROR(fmt, ...) zlog_error(zlog_cat, fmt, ##__VA_ARGS__)

// 状态枚举
enum
{
    UPGRADE_IDLE = 0,
    UPGRADE_RUNNING = 1,
    UPGRADE_SUCCESS = 2,
    UPGRADE_FAILED = 3
};

/* HTTP响应头 */
void print_json_header(void)
{
    printf("Content-type: application/json\r\n");
    printf("Access-Control-Allow-Origin: *\r\n");
    printf("Access-Control-Allow-Methods: GET, POST\r\n");
    printf("\r\n");
}

/* 发送成功响应 */
void send_success(const char *message)
{
    print_json_header();
    printf("{\"success\": true, \"message\": \"%s\"}\n", message ? message : "操作成功");
}

/* 发送错误响应 */
void send_error(const char *message)
{
    print_json_header();
    printf("{\"success\": false, \"message\": \"%s\"}\n", message ? message : "操作失败");
    LOG_ERROR("%s", message ? message : "操作失败");
}

// 获取当前时间字符串
void get_current_time(char *buf, size_t size)
{
    time_t t = time(NULL);
    struct tm *tm = localtime(&t);
    strftime(buf, size, "%Y-%m-%d %H:%M:%S", tm);
}

// 读取状态文件，若文件不存在或解析失败返回空闲状态
int read_status(int *status, int *progress, char *message, size_t msg_size,
                char *filename, size_t fname_size, char *time_str, size_t tsize)
{
    FILE *fp = fopen(UPGRADE_STATUS_FILE, "r");
    if (!fp)
    {
        // 文件不存在，设置默认值并返回0
        *status = UPGRADE_IDLE;
        *progress = 0;
        if (message)
            message[0] = '\0';
        if (filename)
            filename[0] = '\0';
        if (time_str)
            time_str[0] = '\0';
        return 0;
    }

    // 读取文件内容到缓冲区（假设文件大小不超过4KB）
    char buffer[4096];
    size_t len = fread(buffer, 1, sizeof(buffer) - 1, fp);
    buffer[len] = '\0';
    fclose(fp);

    // 解析JSON
    cJSON *root = cJSON_Parse(buffer);
    if (!root)
    {
        // JSON解析失败，设置默认值并返回-1
        *status = UPGRADE_IDLE;
        *progress = 0;
        if (message)
            message[0] = '\0';
        if (filename)
            filename[0] = '\0';
        if (time_str)
            time_str[0] = '\0';
        return -1;
    }

    // 获取各字段
    cJSON *status_item = cJSON_GetObjectItem(root, "status");
    cJSON *progress_item = cJSON_GetObjectItem(root, "progress");
    cJSON *message_item = cJSON_GetObjectItem(root, "message");
    cJSON *filename_item = cJSON_GetObjectItem(root, "filename");
    cJSON *time_item = cJSON_GetObjectItem(root, "time");

    // 检查必需字段是否存在且类型正确
    if (!cJSON_IsNumber(status_item) || !cJSON_IsNumber(progress_item) ||
        !cJSON_IsString(message_item) || !cJSON_IsString(filename_item) ||
        !cJSON_IsString(time_item))
    {
        cJSON_Delete(root);
        *status = UPGRADE_IDLE;
        *progress = 0;
        if (message)
            message[0] = '\0';
        if (filename)
            filename[0] = '\0';
        if (time_str)
            time_str[0] = '\0';
        return -1;
    }

    // 赋值整型字段
    *status = status_item->valueint;
    *progress = progress_item->valueint;

    // 复制字符串字段（注意缓冲区大小）
    if (message)
    {
        strncpy(message, message_item->valuestring, msg_size - 1);
        message[msg_size - 1] = '\0';
    }
    if (filename)
    {
        strncpy(filename, filename_item->valuestring, fname_size - 1);
        filename[fname_size - 1] = '\0';
    }
    if (time_str)
    {
        strncpy(time_str, time_item->valuestring, tsize - 1);
        time_str[tsize - 1] = '\0';
    }

    cJSON_Delete(root);
    return 0;
}

// 写入状态文件
void write_status(int status, int progress, const char *message,
                  const char *filename, const char *time_str)
{
    FILE *fp = fopen(UPGRADE_STATUS_FILE, "w");
    if (!fp)
        return;
    fprintf(fp, "{\"status\":%d,\"progress\":%d,\"message\":\"%s\",\"filename\":\"%s\",\"time\":\"%s\"}",
            status, progress, message, filename ? filename : "", time_str ? time_str : "");
    fclose(fp);
}
/*状态查询接口*/
int get_upgrade_status(void)
{
    LOG_DEBUG("轮询升级文件状态");
    int status, progress;
    char message[256] = "", filename[128] = "", time_str[64] = "";

    read_status(&status, &progress, message, sizeof(message),
                filename, sizeof(filename), time_str, sizeof(time_str));

    print_json_header();
    printf("{\"success\":true,\"status\":%d,\"progress\":%d,\"message\":\"%s\",\"filename\":\"%s\",\"time\":\"%s\"}",
           status, progress, message, filename, time_str);
    return 0;
}

/* 从cJSON获取字符串（带默认值） */
const char *cjson_get_string(cJSON *obj, const char *key, const char *default_val)
{
    cJSON *item = cJSON_GetObjectItem(obj, key);
    if (item && cJSON_IsString(item) && item->valuestring)
    {
        return item->valuestring;
    }
    return default_val;
}

/* 从cJSON获取整数（带默认值） */
int cjson_get_int(cJSON *obj, const char *key, int default_val)
{
    cJSON *item = cJSON_GetObjectItem(obj, key);
    if (item && cJSON_IsNumber(item))
    {
        return item->valueint;
    }
    return default_val;
}

/* 总线配置转换为JSON */
void bus_config_to_json(const bus_config_t *config, char *json, size_t json_size)
{
    cJSON *root = cJSON_CreateObject();
    cJSON_AddBoolToObject(root, "success", 1);

    cJSON *config_obj = cJSON_CreateObject();
    cJSON *comm_rows = cJSON_CreateArray();

    for (int i = 0; i < config->row_count && i < MAX_COMM_ROWS; i++)
    {
        const comm_row_t *row = &config->rows[i];
        if (row->type == BUS_TYPE_UNKNOWN)
            continue;

        cJSON *row_obj = cJSON_CreateObject();
        cJSON_AddStringToObject(row_obj, "id", "placeholder");
        cJSON_AddStringToObject(row_obj, "type", bus_type_to_string(row->type));
        cJSON_AddStringToObject(row_obj, "typeName", row->type_name);
        cJSON_AddNumberToObject(row_obj, "port", row->port);

        // 确保间隔值至少为100
        int internal = row->internal;
        cJSON_AddNumberToObject(row_obj, "internal", internal);

        // 添加IP（如果是网口）
        if (row->type != BUS_TYPE_RS485 && strlen(row->ip) > 0)
        {
            cJSON_AddStringToObject(row_obj, "ip", row->ip);
        }

        // 添加extra_params（如果是RS485）
        if (row->type == BUS_TYPE_RS485 && strlen(row->extra_params) > 0)
        {
            cJSON_AddStringToObject(row_obj, "extra_params", row->extra_params);
        }

        // 添加装置数组
        cJSON *devices = cJSON_CreateArray();
        for (int j = 0; j < row->device_count && j < MAX_DEVICES_PER_ROW; j++)
        {
            const device_t *dev = &row->devices[j];
            cJSON *dev_obj = cJSON_CreateObject();

            char dev_id_str[32];
            snprintf(dev_id_str, sizeof(dev_id_str), "dev_%d", dev->id);
            cJSON_AddStringToObject(dev_obj, "id", dev_id_str);
            cJSON_AddNumberToObject(dev_obj, "module", dev->module);
            cJSON_AddNumberToObject(dev_obj, "order", dev->order);
            cJSON_AddStringToObject(dev_obj, "address", dev->address);
            cJSON_AddStringToObject(dev_obj, "name", dev->name);
            cJSON_AddStringToObject(dev_obj, "protocol", protocol_type_to_string(dev->protocol));
            cJSON_AddStringToObject(dev_obj, "protocolName", protocol_type_to_display_name(dev->protocol));

            // 协议模块
            const char *module_str = "";
            if (dev->module == MODULE_PXR_20)
            {
                module_str = "PXR-20";
            }
            else if (dev->module == MODULE_PXR_25)
            {
                module_str = "PXR-25";
            }
            cJSON_AddStringToObject(dev_obj, "protocolModule", module_str);

            // 模板
            cJSON_AddStringToObject(dev_obj, "template", template_type_to_string(dev->template_type));

            cJSON_AddItemToArray(devices, dev_obj);
        }
        cJSON_AddItemToObject(row_obj, "devices", devices);

        // 更新ID（包含行号）
        char id_str[32];
        snprintf(id_str, sizeof(id_str), "row_%d", row->id);
        cJSON_ReplaceItemInObject(row_obj, "id", cJSON_CreateString(id_str));

        cJSON_AddItemToArray(comm_rows, row_obj);
    }

    cJSON_AddItemToObject(config_obj, "commRows", comm_rows);
    cJSON_AddItemToObject(root, "config", config_obj);

    char *json_str = cJSON_PrintUnformatted(root);
    if (json_str)
    {
        strncpy(json, json_str, json_size - 1);
        json[json_size - 1] = '\0';
        free(json_str);
    }

    cJSON_Delete(root);
}

/* 读取总线配置 */
int bus_get_config(void)
{
    LOG_INFO("接收到总线配置读取请求");

    bus_config_t config;
    memset(&config, 0, sizeof(config));

    if (load_bus_config(&config) != 0)
    {
        LOG_ERROR("加载总线配置失败");
        send_error("加载配置失败");
        return 1;
    }

    char json[65536];
    bus_config_to_json(&config, json, sizeof(json));

    print_json_header();
    printf("%s\n", json);

    LOG_INFO("总线配置读取成功，共%d行", config.row_count);
    LOG_DEBUG("总线配置: %s", json);

    return 0;
}

/* 保存总线配置 */
int bus_set_config(void)
{
    LOG_INFO("接收到总线配置保存请求");

    char *content_length = getenv("CONTENT_LENGTH");
    if (!content_length)
    {
        send_error("请求数据为空");
        return 1;
    }

    int len = atoi(content_length);
    if (len > MAX_POST_SIZE)
    {
        send_error("数据过大");
        return 1;
    }

    char *buffer = malloc(len + 1);
    if (!buffer)
    {
        send_error("内存分配失败");
        return 1;
    }

    fread(buffer, 1, len, stdin);
    buffer[len] = '\0';

    LOG_DEBUG("接收到的JSON数据: %s", buffer);

    // 使用cJSON解析
    cJSON *root = cJSON_Parse(buffer);
    free(buffer);

    if (!root)
    {
        LOG_ERROR("JSON解析失败: %s", cJSON_GetErrorPtr());
        send_error("无效的JSON数据");
        return 1;
    }

    // 构建配置结构
    bus_config_t config;
    memset(&config, 0, sizeof(config));

    cJSON *config_obj = cJSON_GetObjectItem(root, "config");
    if (!config_obj)
    {
        cJSON_Delete(root);
        send_error("无效的请求数据：缺少config字段");
        return 1;
    }

    cJSON *comm_rows = cJSON_GetObjectItem(config_obj, "commRows");
    if (!comm_rows || !cJSON_IsArray(comm_rows))
    {
        cJSON_Delete(root);
        send_error("无效的请求数据：缺少commRows数组");
        return 1;
    }

    int row_count = cJSON_GetArraySize(comm_rows);
    if (row_count > MAX_COMM_ROWS)
    {
        row_count = MAX_COMM_ROWS;
    }

    // 解析每个通讯行
    for (int i = 0; i < row_count; i++)
    {
        cJSON *row_json = cJSON_GetArrayItem(comm_rows, i);
        if (!row_json)
            continue;

        comm_row_t *row = &config.rows[config.row_count];

        // 解析行ID
        const char *id_str = cjson_get_string(row_json, "id", "");
        if (strncmp(id_str, "row_", 4) == 0)
        {
            row->id = atoi(id_str + 4);
        }

        // 解析总线类型
        const char *type_str = cjson_get_string(row_json, "type", "");
        row->type = parse_bus_type(type_str);
        strcpy(row->type_name, bus_type_to_display_name(row->type));

        // 解析IP地址（仅网口）
        if (row->type != BUS_TYPE_RS485)
        {
            const char *ip_str = cjson_get_string(row_json, "ip", "");
            if (strlen(ip_str) > 0)
            {
                strncpy(row->ip, ip_str, sizeof(row->ip) - 1);
            }
            else
            {
                strcpy(row->ip, "168.128.1.1");
            }
        }

        // 解析端口和间隔
        row->port = cjson_get_int(row_json, "port", 502);
        row->internal = cjson_get_int(row_json, "internal", 300);

        // 解析extra_params（RS485串口参数）
        if (row->type == BUS_TYPE_RS485)
        {
            const char *extra_str = cjson_get_string(row_json, "extra_params", "");
            if (strlen(extra_str) > 0)
            {
                strncpy(row->extra_params, extra_str, sizeof(row->extra_params) - 1);
            }
        }

        // 解析装置
        cJSON *devices = cJSON_GetObjectItem(row_json, "devices");
        if (devices && cJSON_IsArray(devices))
        {
            int dev_count = cJSON_GetArraySize(devices);
            if (dev_count > MAX_DEVICES_PER_ROW)
            {
                dev_count = MAX_DEVICES_PER_ROW;
            }

            for (int j = 0; j < dev_count; j++)
            {
                cJSON *dev_json = cJSON_GetArrayItem(devices, j);
                if (!dev_json)
                    continue;

                device_t *dev = &row->devices[row->device_count];

                // 解析装置ID
                const char *dev_id_str = cjson_get_string(dev_json, "id", "");
                if (strncmp(dev_id_str, "dev_", 4) == 0)
                {
                    dev->id = atoi(dev_id_str + 4);
                }

                // 解析其他字段
                dev->order = cjson_get_int(dev_json, "order", j + 1);

                const char *addr_str = cjson_get_string(dev_json, "address", "");
                strncpy(dev->address, addr_str, sizeof(dev->address) - 1);

                const char *name_str = cjson_get_string(dev_json, "name", "");
                strncpy(dev->name, name_str, sizeof(dev->name) - 1);

                // 解析协议
                const char *proto_str = cjson_get_string(dev_json, "protocol", "modbusmaster");
                dev->protocol = parse_protocol_type(proto_str);

                // 解析模板
                const char *tmpl_str = cjson_get_string(dev_json, "template", "");
                dev->template_type = parse_template_type(tmpl_str);

                // 根据模板确定模块
                dev->module = template_to_module(dev->template_type);

                row->device_count++;
            }
        }

        config.row_count++;
    }

    cJSON_Delete(root);

    LOG_INFO("解析完成，共%d个通讯行", config.row_count);

    // 保存配置
    if (save_bus_config(&config) != 0)
    {
        send_error("保存配置失败");
        return 1;
    }

    send_success("总线配置保存成功");
    LOG_INFO("总线配置保存成功");
    return 0;
}

/* 从配置文件中获取 link 状态 */
int bus_get_status(void)
{
    FILE *fp = fopen(COM_STATUS_FILE, "r");
    if (!fp)
    {
        LOG_ERROR("bus_get_status: failed to open config file: %s", COM_STATUS_FILE);
        print_json_header();
        printf("{\"success\":false,\"message\":\"Cannot open config file\"}");
        return -1;
    }

    char line[256];
    int in_com_status = 0;
    int link = -1;

    while (fgets(line, sizeof(line), fp))
    {
        line[strcspn(line, "\n")] = 0;

        if (line[0] == '[')
        {
            in_com_status = (strcmp(line, "[com_status]") == 0);
            continue;
        }

        if (in_com_status)
        {
            char *eq = strchr(line, '=');
            if (eq)
            {
                *eq = '\0';
                char *key = line;
                char *value = eq + 1;
                while (*key == ' ')
                    key++;
                char *end = key + strlen(key) - 1;
                while (end > key && (*end == ' ' || *end == '\t'))
                    *end-- = '\0';
                while (*value == ' ')
                    value++;

                if (strcmp(key, "link") == 0)
                {
                    link = atoi(value);
                    break;
                }
            }
        }
    }
    fclose(fp);

    print_json_header();
    if (link == -1)
    {
        LOG_WARN("bus_get_status: link not found in [com_status] section");
        printf("{\"success\":false,\"message\":\"link parameter not found\"}");
    }
    else
    {
        printf("{\"success\":true,\"link\":%d}", link);
        LOG_INFO("bus_get_status: link=%d", link);
    }
    return 0;
}

// 返回静态缓冲区中的日期字符串，格式 YYYY-MM-DD
const char *get_current_date_str(void)
{
    static char buf[11];
    time_t t = time(NULL);
    struct tm *tm = localtime(&t);
    strftime(buf, sizeof(buf), "%Y-%m-%d", tm);
    return buf;
}

/* 下载 /mynand/log 目录打包文件 */
int download_61850log(void)
{
    // 检查目录是否存在
    struct stat st;
    if (stat(LOG_PATH_61850, &st) != 0 || !S_ISDIR(st.st_mode))
    {
        print_json_header();
        printf("{\"success\":false,\"message\":\"日志目录不存在\"}");
        return 1;
    }
    // 发送下载头
    printf("Content-Type: application/x-gzip\r\n");
    printf("Content-Disposition: attachment; filename=\"logs_%s.tar.gz\"\r\n", get_current_date_str());
    printf("\r\n");
    fflush(stdout);

    // 执行打包压缩命令（注意：此时错误会写入 stderr，可能被 Web 服务器记录，但不会影响 HTTP 响应）
    FILE *pipe = popen("tar -cf - -C /mynand log | gzip -c", "r");
    if (!pipe)
    {
        // 命令启动失败，只能记录错误，无法通知客户端
        fprintf(stderr, "无法执行打包命令\n");
        return 1;
    }

    char buf[8192];
    size_t n;
    while ((n = fread(buf, 1, sizeof(buf), pipe)) > 0)
    {
        fwrite(buf, 1, n, stdout);
    }

    int status = pclose(pipe);
    if (status != 0)
    {
        fprintf(stderr, "打包命令失败，退出码: %d\n", status);
        // 如果已输出部分数据，无法补救；但可以尝试在最后写入一个错误标记？不可行
    }

    return 0;
}

static uint32_t crc_table[256];
void crc32_init()
{
    for (uint32_t i = 0; i < 256; i++)
    {
        uint32_t crc = i;

        for (int j = 0; j < 8; j++)
        {
            if (crc & 1)
                crc = (crc >> 1) ^ 0xEDB88320;
            else
                crc >>= 1;
        }

        crc_table[i] = crc;
    }
}

uint32_t crc32_file(FILE *fp)
{
    uint8_t buf[4096];

    uint32_t crc = 0xFFFFFFFF;

    int n;

    while ((n = fread(buf, 1, sizeof(buf), fp)) > 0)
    {
        for (int i = 0; i < n; i++)
        {
            crc = crc_table[(crc ^ buf[i]) & 0xFF] ^ (crc >> 8);
        }
    }

    return crc ^ 0xFFFFFFFF;
}
/**
 * 获取指定路径所在文件系统的可用空间（字节）
 * 返回 -1 表示获取失败
 */
static long long get_free_space(const char *path)
{
    struct statvfs stat;
    if (statvfs(path, &stat) != 0)
    {
        return -1;
    }
    // f_frsize 是块大小，f_bfree 是可用块数
    return (long long)stat.f_frsize * stat.f_bfree;
}
/* 上传固件 */
int upload_firmware(void)
{
    char *query = getenv("QUERY_STRING");
    char *len_str = getenv("CONTENT_LENGTH");
    char *name = getenv("HTTP_X_FILENAME");

    if (!query)
    {
        send_error("no query"); // 假设此函数返回 JSON 错误
        return -1;
    }

    if (!name || strlen(name) == 0)
    {
        send_error("no filename");
        return -1;
    }

    /* 防止路径攻击 */
    if (strstr(name, "..") || strchr(name, '/'))
    {
        send_error("invalid filename");
        return -1;
    }

    mkdir(FIRMWARE_DIR, 0755);

    char filepath[256];
    snprintf(filepath, sizeof(filepath), "%s/%s", FIRMWARE_DIR, name);

    /* chunk上传 */
    if (strstr(query, "chunk"))
    {
        if (!len_str)
        {
            send_error("no data");
            return -1;
        }

        long len = atol(len_str);

        if (len <= 0 || len > MAX_CHUNK_SIZE)
        {
            LOG_ERROR("chunk size invalid: %ld", len);
            send_error("chunk too large");
            return -1;
        }

        // ========== 获取前端期望的偏移量 ==========
        char *expected_offset_str = getenv("HTTP_X_EXPECTED_OFFSET");
        long expected_offset = (expected_offset_str != NULL) ? atol(expected_offset_str) : -1;

        /* 获取当前文件大小 = 已上传大小 */
        long current_size = 0;

        struct stat st;
        if (stat(filepath, &st) == 0)
        {
            current_size = st.st_size;
        }

        // ========== 清理不一致的残留文件 ==========
        if (current_size > 0 && expected_offset >= 0 && current_size != expected_offset)
        {
            LOG_INFO("文件大小 %ld 与前端期望偏移 %ld 不一致，删除残留文件", current_size, expected_offset);
            unlink(filepath);
            current_size = 0;
        }

        // 如果期望续传但后端没有文件，返回错误让前端重头开始
        if (current_size == 0 && expected_offset > 0)
        {
            send_error("no file to resume");
            return -1;
        }

        /* 计算chunk序号 */
        long chunk_index = current_size / MAX_CHUNK_SIZE + 1;

        LOG_INFO("开始接收分片: index=%ld chunk_size=%ld current_file_size=%ld file=%s",
                 chunk_index, len, current_size, filepath);

        /* 检查磁盘空间 */
        long long free_space = get_free_space(FIRMWARE_DIR);

        if (free_space >= 0 && free_space < len + 4096)
        {
            LOG_ERROR("磁盘空间不足: free=%lld need=%ld", free_space, len);

            unlink(filepath);

            send_error("no space left on device");
            return -1;
        }

        FILE *fp = fopen(filepath, "ab");
        if (!fp)
        {
            LOG_ERROR("open file fail: %s", filepath);
            send_error("open file fail");
            return -1;
        }

        char *buf = malloc(BUFFER_SIZE);
        if (!buf)
        {
            fclose(fp);
            LOG_ERROR("malloc fail");
            send_error("no memory");
            return -1;
        }

        long received = 0;

        while (received < len)
        {
            int need = BUFFER_SIZE;

            if (len - received < need)
                need = len - received;

            int n = fread(buf, 1, need, stdin);

            if (n <= 0)
            {
                LOG_ERROR("stdin read fail");
                break;
            }

            if (fwrite(buf, 1, n, fp) != n)
            {
                free(buf);
                fclose(fp);

                LOG_ERROR("write fail");

                unlink(filepath);

                send_error("write fail");
                return -1;
            }

            received += n;
        }

        free(buf);

        fflush(fp);

        long offset = ftell(fp);

        fclose(fp);

        if (received != len)
        {
            LOG_ERROR("upload interrupted: expect=%ld recv=%ld", len, received);

            unlink(filepath);

            send_error("upload interrupted");
            return -1;
        }

        LOG_INFO("分片上传完成: index=%ld chunk=%ld total_uploaded=%ld",
                 chunk_index, len, offset);

        /* 返回当前写入偏移量 */
        print_json_header();
        printf("{\"success\":true,\"offset\":%ld}", offset);

        return 0;
    }

    /* finish 处理（CRC校验） */
    if (strstr(query, "finish"))
    {
        char *crc_str = getenv("HTTP_X_CRC32");
        if (!crc_str)
        {
            send_error("no CRC");
            return -1;
        }

        uint32_t expect_crc = strtoul(crc_str, NULL, 16);
        FILE *fp = fopen(filepath, "rb");
        if (!fp)
        {
            send_error("file not found");
            return -1;
        }

        crc32_init();
        uint32_t real_crc = crc32_file(fp);
        fclose(fp);

        if (real_crc != expect_crc)
        {
            unlink(filepath); // CRC 错误，删除无效文件
            send_error("CRC error");
            return -1;
        }
        /* 验证完成 */
        LOG_INFO("固件上传完成: %s", filepath);

        // 检查是否已有升级任务运行
        int status, progress;
        char msg[256], fname[128], tstr[64];
        if (read_status(&status, &progress, msg, sizeof(msg), fname, sizeof(fname), tstr, sizeof(tstr)) == 0 &&
            status == UPGRADE_RUNNING)
        {
            send_error("upgrade already in progress");
            return -1;
        }

        // 重命名为固定文件名
        // char fixed_path[256];
        // snprintf(fixed_path, sizeof(fixed_path), "%s/current.bin", FIRMWARE_DIR);
        // rename(filepath, fixed_path);
        system("sync");
        chmod(filepath, 0755);

        pid_t pid = fork();
        if (pid == 0)
        {
            // 子进程
            // 获取当前时间作为开始时间
            char start_time[64];
            get_current_time(start_time, sizeof(start_time));

            // 写入初始状态（升级中）
            write_status(UPGRADE_RUNNING, 0, "升级中...", name, start_time);

            // 执行升级（阻塞）
            char cmd[256];
            snprintf(cmd, sizeof(cmd), "%s", filepath);
            int ret = system(cmd);

            // 等待 2 秒，给 bin 程序写入最终状态的机会（如果它写的话）
            sleep(2);

            // 检查 bin 程序是否已经更新了状态
            int new_status, new_progress;
            char new_msg[256], new_fname[128], new_time[64];
            if (read_status(&new_status, &new_progress, new_msg, sizeof(new_msg),
                            new_fname, sizeof(new_fname), new_time, sizeof(new_time)) == 0 &&
                new_status != UPGRADE_RUNNING)
            {
                // bin 已经写了最终状态，不需要再写
                // 但确保 progress 为 100 等
            }
            else
            {
                // bin 没有写最终状态，由我们补写
                if (ret == 0)
                {
                    write_status(UPGRADE_SUCCESS, 100, "升级成功（由系统补录）", name, start_time);
                }
                else
                {
                    char fail_msg[128];
                    snprintf(fail_msg, sizeof(fail_msg), "升级失败，返回码 %d", ret);
                    write_status(UPGRADE_FAILED, 100, fail_msg, name, start_time);
                }
            }

            // 删除固件文件
            unlink(filepath);
            system("sync");
            exit(0);
        }
        else if (pid < 0)
        {
            send_error("cannot start upgrade process");
            return -1;
        }

        // 父进程立即返回
        print_json_header();
        printf("{\"success\":true,\"message\":\"upgrade started\"}");
        return 0;
    }

    send_error("invalid request");
    return -1;
}

/* 验证密码 - 支持MD5加密的密码文件 */
static int verify_password(const char *username, const char *password, char *stored_hash, size_t hash_size)
{
    FILE *fp = fopen(PASSWD_FILE, "r");
    if (!fp)
    {
        /* 尝试备用路径 */
        fp = fopen(FALLBACK_PASSWD_FILE, "r");
        if (!fp)
        {
            LOG_WARN("无法打开密码文件: %s", PASSWD_FILE);
            return -1;
        }
    }

    char line[MAX_LINE_LENGTH];
    int found = 0;

    while (fgets(line, sizeof(line), fp))
    {
        /* 跳过注释和空行 */
        if (line[0] == '#' || line[0] == '\n' || line[0] == '\r')
        {
            continue;
        }

        /* 去除行尾换行符 */
        line[strcspn(line, "\r\n")] = '\0';

        /* 查找冒号分隔符 */
        char *colon = strchr(line, ':');
        if (!colon)
        {
            continue;
        }

        *colon = '\0';
        const char *file_user = line;
        const char *file_hash = colon + 1;

        if (strcmp(file_user, username) == 0)
        {
            strncpy(stored_hash, file_hash, hash_size - 1);
            stored_hash[hash_size - 1] = '\0';
            found = 1;
            break;
        }
    }

    fclose(fp);
    return found ? 0 : -1;
}

/* 用户登录 */
int user_login(void)
{
    char *content_length = getenv("CONTENT_LENGTH");
    if (!content_length)
    {
        send_error("请求数据为空");
        return 1;
    }

    int len = atoi(content_length);
    if (len <= 0 || len > MAX_POST_SIZE)
    {
        send_error("无效的数据长度");
        return 1;
    }

    char *buffer = malloc(len + 1);
    if (!buffer)
    {
        send_error("内存分配失败");
        return 1;
    }

    size_t read_len = fread(buffer, 1, len, stdin);
    buffer[read_len] = '\0';

    LOG_DEBUG("登录请求数据: %s", buffer);

    char username[64] = {0};
    char password[64] = {0};

    /* URL解码 */
    char *p = buffer;
    char decoded[256] = {0};
    int j = 0;

    while (*p && j < sizeof(decoded) - 1)
    {
        if (*p == '%' && p[1] && p[2])
        {
            /* URL解码 %XX */
            int hex1 = p[1] >= 'A' ? (p[1] & 0xDF) - 'A' + 10 : p[1] - '0';
            int hex2 = p[2] >= 'A' ? (p[2] & 0xDF) - 'A' + 10 : p[2] - '0';
            decoded[j++] = (hex1 << 4) | hex2;
            p += 3;
        }
        else if (*p == '+')
        {
            /* URL解码 + 为空格 */
            decoded[j++] = ' ';
            p++;
        }
        else
        {
            decoded[j++] = *p++;
        }
    }
    decoded[j] = '\0';

    /* 解析用户名和密码 */
    char *username_ptr = strstr(decoded, "username=");
    char *password_ptr = strstr(decoded, "&password=");

    if (username_ptr)
    {
        username_ptr += 9; /* 跳过 "username=" */
        if (password_ptr)
        {
            size_t username_len = password_ptr - username_ptr;
            if (username_len >= sizeof(username))
            {
                username_len = sizeof(username) - 1;
            }
            strncpy(username, username_ptr, username_len);

            password_ptr += 10; /* 跳过 "&password=" */
            strncpy(password, password_ptr, sizeof(password) - 1);
        }
    }

    free(buffer);

    if (strlen(username) == 0 || strlen(password) == 0)
    {
        send_error("用户名或密码不能为空");
        return 1;
    }

    LOG_INFO("用户登录请求: %s", username);

    /* 从密码文件验证 */
    char stored_hash[256] = {0};
    int result = verify_password(username, password, stored_hash, sizeof(stored_hash));
    LOG_DEBUG("密码验证结果: %d, stored_hash: %s", result, stored_hash);

    if (result == 0 && strlen(stored_hash) > 0)
    {
        /* 检查密码格式 */
        int valid = 0;

        if (strncmp(stored_hash, "$1$", 3) == 0)
        {
            /* MD5加密格式 (htpasswd -m) */
            char *encrypted = crypt(password, stored_hash);
            LOG_DEBUG("MD5验证 - 输入密码: %s, 存储哈希: %s, 计算结果: %s", password, stored_hash, encrypted ? encrypted : "NULL");
            if (encrypted && strcmp(encrypted, stored_hash) == 0)
            {
                valid = 1;
                LOG_DEBUG("MD5密码验证成功");
            }
            else
            {
                LOG_DEBUG("MD5密码验证失败 - encrypted=%s", encrypted ? encrypted : "NULL");
            }
        }
        else if (strncmp(stored_hash, "$apr1$", 6) == 0) // 新增 Apache MD5 支持
        {
            /* Apache MD5 加密格式 (htpasswd -m 默认) */
            char *encrypted = crypt(password, stored_hash);
            LOG_DEBUG("APR1验证 - 输入密码: %s, 存储哈希: %s, 计算结果: %s", password, stored_hash, encrypted ? encrypted : "NULL");
            if (encrypted && strcmp(encrypted, stored_hash) == 0)
            {
                valid = 1;
                LOG_DEBUG("APR1密码验证成功");
            }
            else
            {
                LOG_DEBUG("APR1密码验证失败");
            }
        }
        else if (strncmp(stored_hash, "$2a$", 4) == 0 || strncmp(stored_hash, "$2b$", 4) == 0 || strncmp(stored_hash, "$2y$", 4) == 0)
        {
            /* bcrypt格式 (htpasswd -B) - 需要额外库支持 */
            /* 简化处理: bcrypt不支持直接用crypt比较，这里返回错误提示 */
            LOG_ERROR("bcrypt加密格式不支持，请使用MD5加密(htpasswd -m)");
            valid = 0;
        }
        else if (strncmp(stored_hash, "$5$", 3) == 0 || strncmp(stored_hash, "$6$", 3) == 0)
        {
            /* SHA-256/SHA-512加密 */
            char *encrypted = crypt(password, stored_hash);
            LOG_DEBUG("SHA验证 - 输入密码: %s, 存储哈希: %s, 计算结果: %s", password, stored_hash, encrypted ? encrypted : "NULL");
            if (encrypted && strcmp(encrypted, stored_hash) == 0)
            {
                valid = 1;
                LOG_DEBUG("SHA密码验证成功");
            }
            else
            {
                LOG_DEBUG("SHA密码验证失败");
            }
        }
        else if (strncmp(stored_hash, "{SHA}", 5) == 0)
        {
            /* SHA1加密 (htpasswd -s) - 需要额外处理 */
            LOG_ERROR("SHA1加密格式不支持，请使用MD5加密(htpasswd -m)");
            valid = 0;
        }
        else
        {
            /* 明文密码（不安全，仅用于测试） */
            LOG_DEBUG("明文验证 - 输入密码: %s, 存储密码: %s", password, stored_hash);
            if (strcmp(password, stored_hash) == 0)
            {
                valid = 1;
                LOG_DEBUG("明文密码验证成功");
            }
            else
            {
                LOG_DEBUG("明文密码验证失败");
            }
        }

        LOG_DEBUG("最终验证结果 valid=%d", valid);
        if (valid)
        {
            print_json_header();
            printf("{\"success\": true, \"message\": \"登录成功\", \"token\": \"emu2000_token_%s\"}\n", username);
            LOG_INFO("用户 %s 登录成功", username);
            return 0;
        }
        else
        {
            LOG_WARN("密码文件验证失败，尝试硬编码密码");
        }
    }
    else
    {
        LOG_DEBUG("密码文件查找失败: result=%d, hash长度=%zu", result, strlen(stored_hash));
    }

    /*
     * 密码文件验证未通过（用户名不存在、密码错误或文件无法打开）
     * 作为开发测试备用，允许使用硬编码 admin/admin 登录
     * 注意：生产环境部署前必须删除此逻辑或修改默认密码！
     */
    if (strcmp(username, "admin") == 0 && strcmp(password, "admin") == 0)
    {
        LOG_WARN("使用硬编码密码登录: %s", username);
        print_json_header();
        printf("{\"success\": true, \"message\": \"登录成功\", \"token\": \"emu2000_token_%s\"}\n", username);
        LOG_INFO("用户 %s 登录成功", username);
        return 0;
    }

    LOG_WARN("用户 %s 登录失败", username);
    send_error("用户名或密码错误");
    return 1;
}

/* 修改密码 */
int change_password(void)
{
    char *content_length = getenv("CONTENT_LENGTH");
    if (!content_length)
    {
        send_error("请求数据为空");
        return 1;
    }

    int len = atoi(content_length);
    if (len <= 0 || len > MAX_POST_SIZE)
    {
        send_error("无效的数据长度");
        return 1;
    }

    char *buffer = malloc(len + 1);
    if (!buffer)
    {
        send_error("内存分配失败");
        return 1;
    }

    size_t read_len = fread(buffer, 1, len, stdin);
    buffer[read_len] = '\0';

    LOG_DEBUG("修改密码请求数据: %s", buffer);

    /* URL解码 */
    char *p = buffer;
    char decoded[256] = {0};
    int j = 0;

    while (*p && j < sizeof(decoded) - 1)
    {
        if (*p == '%' && p[1] && p[2])
        {
            int hex1 = p[1] >= 'A' ? (p[1] & 0xDF) - 'A' + 10 : p[1] - '0';
            int hex2 = p[2] >= 'A' ? (p[2] & 0xDF) - 'A' + 10 : p[2] - '0';
            decoded[j++] = (hex1 << 4) | hex2;
            p += 3;
        }
        else if (*p == '+')
        {
            decoded[j++] = ' ';
            p++;
        }
        else
        {
            decoded[j++] = *p++;
        }
    }
    decoded[j] = '\0';

    /* 解析旧密码和新密码 */
    char username[64] = {0};
    char old_pass[64] = {0};
    char new_pass[64] = {0};

    // 解析 username
    char *username_ptr = strstr(decoded, "username=");
    if (username_ptr)
    {
        username_ptr += 9; // 跳过 "username="
        char *end = strchr(username_ptr, '&');
        if (end)
        {
            size_t len = end - username_ptr;
            if (len >= sizeof(username))
                len = sizeof(username) - 1;
            strncpy(username, username_ptr, len);
        }
        else
        {
            strncpy(username, username_ptr, sizeof(username) - 1);
        }
    }

    char *old_ptr = strstr(decoded, "old_password=");
    char *new_ptr = strstr(decoded, "&new_password=");

    if (old_ptr && new_ptr)
    {
        old_ptr += 13; /* 跳过 "old_password=" */
        size_t old_len = new_ptr - old_ptr;
        if (old_len >= sizeof(old_pass))
        {
            old_len = sizeof(old_pass) - 1;
        }
        strncpy(old_pass, old_ptr, old_len);

        new_ptr += 14; /* 跳过 "&new_password=" */
        strncpy(new_pass, new_ptr, sizeof(new_pass) - 1);
    }

    free(buffer);

    // 字段非空校验
    if (strlen(username) == 0 || strlen(old_pass) == 0 || strlen(new_pass) == 0)
    {
        send_error("用户名、原密码或新密码不能为空");
        return 1;
    }
    if (strlen(new_pass) < 5)
    {
        send_error("新密码长度不能少于5位");
        return 1;
    }
    /* 验证旧密码 */
    char stored_hash[256] = {0};
    int verify_result = verify_password(username, old_pass, stored_hash, sizeof(stored_hash));

    int old_pass_valid = 0;
    if (verify_result == 0 && strlen(stored_hash) > 0)
    {
        if (strncmp(stored_hash, "$1$", 3) == 0)
        {
            char *encrypted = crypt(old_pass, stored_hash);
            if (encrypted && strcmp(encrypted, stored_hash) == 0)
            {
                old_pass_valid = 1;
            }
        }
        else if (strncmp(stored_hash, "$apr1$", 6) == 0) // 新增
        {
            char *encrypted = crypt(old_pass, stored_hash);
            if (encrypted && strcmp(encrypted, stored_hash) == 0)
            {
                old_pass_valid = 1;
            }
        }
        else if (strncmp(stored_hash, "$5$", 3) == 0 || strncmp(stored_hash, "$6$", 3) == 0)
        {
            char *encrypted = crypt(old_pass, stored_hash);
            if (encrypted && strcmp(encrypted, stored_hash) == 0)
            {
                old_pass_valid = 1;
            }
        }
        else if (strcmp(old_pass, stored_hash) == 0)
        {
            old_pass_valid = 1;
        }
    }

    /* 如果密码文件验证失败，检查硬编码密码 */
    if (!old_pass_valid && strcmp(old_pass, "admin") == 0)
    {
        old_pass_valid = 1;
    }

    if (!old_pass_valid)
    {
        send_error("原密码错误");
        return 1;
    }

    /* 生成新密码的MD5哈希 */
    char salt[32];
    snprintf(salt, sizeof(salt), "$1$%08lx%08lx", (unsigned long)random(), (unsigned long)time(NULL));
    char *new_hash = crypt(new_pass, salt);

    if (!new_hash)
    {
        send_error("密码加密失败");
        return 1;
    }

    /* 更新密码文件 */
    FILE *fp_in = fopen(PASSWD_FILE, "r");
    if (!fp_in)
    {
        fp_in = fopen(FALLBACK_PASSWD_FILE, "r");
    }

    /* 创建临时文件 */
    char temp_file[256];
    snprintf(temp_file, sizeof(temp_file), "%s.tmp", PASSWD_FILE);
    FILE *fp_out = fopen(temp_file, "w");
    if (!fp_out)
    {
        if (fp_in)
            fclose(fp_in);
        send_error("无法创建临时文件");
        return 1;
    }

    /* 写入注释 */
    fprintf(fp_out, "# EMU2000 Web 用户密码文件\n");
    fprintf(fp_out, "# 格式: username:encrypted_password\n");
    fprintf(fp_out, "# 使用  mkpasswd -m 命令生成MD5加密密码\n");

    int user_found = 0;
    if (fp_in)
    {
        char line[MAX_LINE_LENGTH];
        while (fgets(line, sizeof(line), fp_in))
        {
            if (line[0] == '#' || line[0] == '\n' || line[0] == '\r')
                continue;

            line[strcspn(line, "\r\n")] = '\0';
            char *colon = strchr(line, ':');
            if (!colon)
                continue;

            *colon = '\0';
            if (strcmp(line, username) == 0)
            {
                fprintf(fp_out, "%s:%s\n", username, new_hash);
                user_found = 1;
            }
            else
            {
                fprintf(fp_out, "%s:%s\n", line, colon + 1);
            }
        }
        fclose(fp_in);
    }

    if (!user_found)
    {
        // 用户不存在（可能被删除），返回错误
        fclose(fp_out);
        remove(temp_file);
        send_error("用户不存在");
        return 1;
    }

    fclose(fp_out);

    /* 替换原文件 */
    if (rename(temp_file, PASSWD_FILE) != 0)
    {
        /* 如果重命名失败，尝试备用路径 */
        if (rename(temp_file, FALLBACK_PASSWD_FILE) != 0)
        {
            remove(temp_file);
            send_error("保存密码文件失败");
            return 1;
        }
    }

    /* 设置合适的权限 */
    chmod(PASSWD_FILE, 0600);

    send_success("密码修改成功");
    LOG_INFO("用户 admin 修改密码成功");
    return 0;
}

/* 系统重启 */
int system_reboot(void)
{
    LOG_INFO("系统重启请求");

    // 返回 JSON
    print_json_header();
    printf("{\"success\":true,\"message\":\"系统即将重启\"}");
    fflush(stdout);

    // 通用重启命令
    system("(sleep 1; /sbin/reboot) &");

    return 0;
}

/* 获取系统信息 */
int get_system_info(void)
{
    print_json_header();
    printf("{\"success\": true, \"data\": {\"version\": \"1.0.0\", \"uptime\": \"10 days\"}}\n");
    return 0;
}

/* 获取硬件配置 */
int hw_get_config(void)
{
    LOG_DEBUG("========获取硬件配置请求");
    print_json_header();

    // 预设最大支持4个网口
    char ip[4][32] = {"", "", "", ""};
    char mask[4][32] = {"", "", "", ""};
    char gateway[4][32] = {"", "", "", ""};
    char ied_name[64] = "";
    char comtrade_type[32] = "";
    char ntp_ip[32] = "";
    char ntp_port[32] = "";
    char ntp_timezone[32] = "";
    char log_netcard[16] = "";
    char log_ip[32] = "";
    char log_port[32] = "";

    // 解析BusLine.ini
    FILE *fp = fopen(BUSLINE_FILE, "r");
    if (fp)
    {
        char line[MAX_LINE_LENGTH];
        char current_section[64] = "";
        int current_idx = -1;
        while (fgets(line, sizeof(line), fp))
        {
            line[strcspn(line, "\r\n")] = '\0';
            if (line[0] == '#' || strlen(line) == 0)
                continue;
            if (line[0] == '[')
            {
                sscanf(line, "[%63[^]]", current_section);
                current_idx = -1;
                continue;
            }
            if (strcmp(current_section, "NetCard") == 0)
            {
                // 先判断是否是 NetCardXX=ethX
                int idx = -1;
                char ifname[16] = "";
                if (sscanf(line, "NetCard%02d=%15s", &idx, ifname) == 2 && idx >= 1 && idx <= 4)
                {
                    current_idx = idx - 1;
                    continue;
                }
                // 只在有 current_idx 有效时才解析参数
                if (current_idx >= 0 && current_idx < 4)
                {
                    if (sscanf(line, "IP%*2d=%31s", ip[current_idx]) == 1)
                    {
                        LOG_DEBUG("[NetCard] eth%d IP=%s", current_idx, ip[current_idx]);
                    }
                    else if (sscanf(line, "SubNetMask%*2d=%31s", mask[current_idx]) == 1)
                    {
                        LOG_DEBUG("[NetCard] eth%d Mask=%s", current_idx, mask[current_idx]);
                    }
                    else if (sscanf(line, "GateWay%*2d=%31s", gateway[current_idx]) == 1)
                    {
                        LOG_DEBUG("[NetCard] eth%d Gateway=%s", current_idx, gateway[current_idx]);
                    }
                }
            }
            else if (strcmp(current_section, "PROJECT") == 0)
            {
                if (strncmp(line, "name=", 5) == 0)
                {
                    strncpy(ied_name, line + 5, sizeof(ied_name) - 1);
                    LOG_DEBUG("[PROJECT] name=%s", ied_name);
                }
            }
            else if (strcmp(current_section, "NtpClient") == 0)
            {
                if (strncmp(line, "ServerIP=", 9) == 0)
                {
                    strncpy(ntp_ip, line + 9, sizeof(ntp_ip) - 1);
                    LOG_DEBUG("[NTPClient] ServerIP=%s", ntp_ip);
                }
                if (strncmp(line, "ServerPort=", 11) == 0)
                {
                    strncpy(ntp_port, line + 11, sizeof(ntp_port) - 1);
                    LOG_DEBUG("[NTPClient] ServerPort=%s", ntp_port);
                }
                if (strncmp(line, "timezone=", 9) == 0)
                {
                    strncpy(ntp_timezone, line + 9, sizeof(ntp_timezone) - 1);
                    LOG_DEBUG("[NTPClient] timezone=%s", ntp_timezone);
                }
            }
            else if (strcmp(current_section, "PRINT_PROTOCOL_MSG") == 0)
            {
                if (strncmp(line, "NetCard=", 8) == 0)
                {
                    strncpy(log_netcard, line + 8, sizeof(log_netcard) - 1);
                    LOG_DEBUG("[PRINT_PROTOCOL_MSG] NetCard=%s", log_netcard);
                }
                if (strncmp(line, "RemoteIP=", 9) == 0)
                {
                    strncpy(log_ip, line + 9, sizeof(log_ip) - 1);
                    LOG_DEBUG("[PRINT_PROTOCOL_MSG] RemoteIP=%s", log_ip);
                }
                if (strncmp(line, "StartPortNum=", 13) == 0)
                {
                    strncpy(log_port, line + 13, sizeof(log_port) - 1);
                    LOG_DEBUG("[PRINT_PROTOCOL_MSG] StartPortNum=%s", log_port);
                }
            }
        }
        fclose(fp);
    }
    else
    {
        LOG_WARN("无法打开BusLine.ini: %s", BUSLINE_FILE);
    }

    // 解析comtrade类型
    fp = fopen(COMTRADE_FILE, "r");
    if (fp)
    {
        char line[MAX_LINE_LENGTH];
        while (fgets(line, sizeof(line), fp))
        {
            line[strcspn(line, "\r\n")] = '\0';
            if (strncmp(line, "data_type=", 10) == 0)
            {
                strncpy(comtrade_type, line + 10, sizeof(comtrade_type) - 1);
                LOG_DEBUG("[comtrade_cfg] data_type=%s", comtrade_type);
            }
        }
        fclose(fp);
    }
    else
    {
        LOG_WARN("无法打开comtrade.conf: %s", COMTRADE_FILE);
    }

    // 构造JSON
    cJSON *root = cJSON_CreateObject();
    cJSON_AddBoolToObject(root, "success", 1);
    cJSON *config = cJSON_CreateObject();
    cJSON *network = cJSON_CreateArray();
    for (int i = 0; i < 4; ++i)
    {
        cJSON *eth = cJSON_CreateObject();
        char ifname[8];
        snprintf(ifname, sizeof(ifname), "eth%d", i);
        cJSON_AddStringToObject(eth, "interface", ifname);
        cJSON_AddStringToObject(eth, "ip", ip[i]);
        cJSON_AddStringToObject(eth, "mask", mask[i]);
        cJSON_AddStringToObject(eth, "gateway", gateway[i]);
        cJSON_AddItemToArray(network, eth);
    }
    cJSON_AddItemToObject(config, "network", network);

    cJSON *serial = cJSON_CreateObject();
    cJSON_AddStringToObject(serial, "port", "rs485");
    cJSON_AddStringToObject(serial, "baud", "115200");
    cJSON_AddStringToObject(serial, "data", "8");
    cJSON_AddStringToObject(serial, "stop", "1");
    cJSON_AddStringToObject(serial, "parity", "none");
    cJSON_AddItemToObject(config, "serial", serial);

    cJSON *iec61850 = cJSON_CreateObject();
    cJSON_AddStringToObject(iec61850, "ied_name", ied_name);
    cJSON_AddItemToObject(config, "iec61850", iec61850);

    cJSON *ntpclient = cJSON_CreateObject();
    cJSON_AddStringToObject(ntpclient, "ServerIP", ntp_ip);
    cJSON_AddStringToObject(ntpclient, "ServerPort", ntp_port);
    cJSON_AddStringToObject(ntpclient, "timezone", ntp_timezone);
    cJSON_AddItemToObject(config, "ntpclient", ntpclient);

    cJSON *comtrade = cJSON_CreateObject();
    cJSON_AddStringToObject(comtrade, "dat_data_type", comtrade_type);
    cJSON_AddItemToObject(config, "comtrade", comtrade);

    cJSON *log = cJSON_CreateObject();
    cJSON_AddStringToObject(log, "log_netcard", log_netcard);
    cJSON_AddStringToObject(log, "log_ip", log_ip);
    cJSON_AddStringToObject(log, "log_port", log_port);
    cJSON_AddItemToObject(config, "log", log);

    cJSON_AddItemToObject(root, "config", config);

    char *json_str = cJSON_Print(root);
    if (json_str)
    {
        LOG_DEBUG("hw_get_config 返回JSON: %s", json_str);
        printf("%s\n", json_str);
        free(json_str);
    }
    cJSON_Delete(root);
    return 0;
}

/* 设置硬件配置 */
int hw_set_config(void)
{
    char *content_length = getenv("CONTENT_LENGTH");
    if (!content_length)
    {
        send_error("请求数据为空");
        return 1;
    }

    int len = atoi(content_length);
    char *buffer = malloc(len + 1);
    fread(buffer, 1, len, stdin);
    buffer[len] = '\0';

    LOG_INFO("接收到硬件配置: %s", buffer);

    /* 使用cJSON解析 */
    cJSON *root = cJSON_Parse(buffer);
    free(buffer);

    if (!root)
    {
        send_error("无效的JSON数据");
        return 1;
    }

    char ip0[16] = "", mask0[16] = "", gateway0[16] = "";
    char ip1[16] = "", mask1[16] = "", gateway1[16] = "";
    char ied_name[64] = "";
    char comtrade_type[32] = "";
    char ntp_ip[32] = "";
    char ntp_port[32] = "";
    char ntp_timezone[32] = "";
    char log_netcard[16] = "";
    char log_ip[32] = "";
    char log_port[32] = "";

    cJSON *network = cJSON_GetObjectItem(root, "network");
    if (network && cJSON_IsArray(network))
    {
        int count = cJSON_GetArraySize(network);
        for (int i = 0; i < count; i++)
        {
            cJSON *eth = cJSON_GetArrayItem(network, i);
            const char *iface = cjson_get_string(eth, "interface", "");
            const char *ip = cjson_get_string(eth, "ip", "");
            const char *mask = cjson_get_string(eth, "mask", "");
            const char *gateway = cjson_get_string(eth, "gateway", "");

            LOG_DEBUG("[hw_set_config] %s: ip=%s, mask=%s, gateway=%s", iface, ip, mask, gateway);

            if (strcmp(iface, "eth0") == 0)
            {
                strncpy(ip0, ip, sizeof(ip0) - 1);
                ip0[sizeof(ip0) - 1] = '\0';
                strncpy(mask0, mask, sizeof(mask0) - 1);
                mask0[sizeof(mask0) - 1] = '\0';
                strncpy(gateway0, gateway, sizeof(gateway0) - 1);
                gateway0[sizeof(gateway0) - 1] = '\0';
            }
            else if (strcmp(iface, "eth1") == 0)
            {
                strncpy(ip1, ip, sizeof(ip1) - 1);
                ip1[sizeof(ip1) - 1] = '\0';
                strncpy(mask1, mask, sizeof(mask1) - 1);
                mask1[sizeof(mask1) - 1] = '\0';
                strncpy(gateway1, gateway, sizeof(gateway1) - 1);
                gateway1[sizeof(gateway1) - 1] = '\0';
            }
        }
    }

    cJSON *iec61850 = cJSON_GetObjectItem(root, "iec61850");
    if (iec61850)
    {
        strncpy(ied_name, cjson_get_string(iec61850, "ied_name", ""), 63);
    }

    cJSON *ntpclient = cJSON_GetObjectItem(root, "ntpclient");
    if (ntpclient)
    {
        strncpy(ntp_ip, cjson_get_string(ntpclient, "ServerIP", ""), 31);
        strncpy(ntp_port, cjson_get_string(ntpclient, "ServerPort", ""), 31);
        strncpy(ntp_timezone, cjson_get_string(ntpclient, "timezone", ""), 31);
    }

    cJSON *comtrade = cJSON_GetObjectItem(root, "comtrade");
    if (comtrade)
    {
        strncpy(comtrade_type, cjson_get_string(comtrade, "dat_data_type", ""), 31);
    }

    cJSON *log = cJSON_GetObjectItem(root, "log");
    if (log)
    {
        strncpy(log_netcard, cjson_get_string(log, "log_netcard", ""), 15);
        strncpy(log_ip, cjson_get_string(log, "log_ip", ""), 31);
        strncpy(log_port, cjson_get_string(log, "log_port", ""), 31);
    }

    FILE *fp_in = fopen(BUSLINE_FILE, "r");
    if (fp_in)
    {
        char lines[4096][MAX_LINE_LENGTH];
        int line_count = 0;
        char line[MAX_LINE_LENGTH];
        while (fgets(line, sizeof(line), fp_in) && line_count < 4096)
        {
            strncpy(lines[line_count], line, MAX_LINE_LENGTH - 1);
            lines[line_count][MAX_LINE_LENGTH - 1] = '\0';
            line_count++;
        }
        fclose(fp_in);

        char tmp_file[256];
        snprintf(tmp_file, sizeof(tmp_file), "%s.tmp", BUSLINE_FILE);
        FILE *fp_out = fopen(tmp_file, "w");
        if (fp_out)
        {
            char current_section[64] = "";
            for (int i = 0; i < line_count; i++)
            {
                char *trimmed = lines[i];
                while (*trimmed == ' ' || *trimmed == '\t')
                    trimmed++;

                if (trimmed[0] == '[')
                {
                    char *end = strchr(trimmed, ']');
                    if (end)
                    {
                        *end = '\0';
                        strcpy(current_section, trimmed + 1);
                        *end = ']';
                    }
                }

                if (strcmp(current_section, "NetCard") == 0)
                {
                    if (strncmp(lines[i], "IP01=", 5) == 0)
                    {
                        fprintf(fp_out, "IP01=%s\n", ip0[0] ? ip0 : "");
                        continue;
                    }
                    else if (strncmp(lines[i], "SubNetMask01=", 13) == 0)
                    {
                        fprintf(fp_out, "SubNetMask01=%s\n", mask0[0] ? mask0 : "255.255.255.0");
                        continue;
                    }
                    else if (strncmp(lines[i], "GateWay01=", 10) == 0)
                    {
                        fprintf(fp_out, "GateWay01=%s\n", gateway0[0] ? gateway0 : "");
                        continue;
                    }
                    else if (strncmp(lines[i], "IP02=", 5) == 0)
                    {
                        fprintf(fp_out, "IP02=%s\n", ip1[0] ? ip1 : "");
                        continue;
                    }
                    else if (strncmp(lines[i], "SubNetMask02=", 13) == 0)
                    {
                        fprintf(fp_out, "SubNetMask02=%s\n", mask1[0] ? mask1 : "255.255.255.0");
                        continue;
                    }
                    else if (strncmp(lines[i], "GateWay02=", 10) == 0)
                    {
                        fprintf(fp_out, "GateWay02=%s\n", gateway1[0] ? gateway1 : "");
                        continue;
                    }
                }
                else if (strcmp(current_section, "PROJECT") == 0)
                {
                    if (strncmp(lines[i], "name=", 5) == 0)
                    {
                        fprintf(fp_out, "name=%s\n", ied_name[0] ? ied_name : "test");
                        continue;
                    }
                }
                else if (strcmp(current_section, "NtpClient") == 0)
                {
                    if (strncmp(lines[i], "ServerIP=", 9) == 0)
                    {
                        fprintf(fp_out, "ServerIP=%s\n", ntp_ip[0] ? ntp_ip : "");
                        continue;
                    }
                    if (strncmp(lines[i], "ServerPort=", 11) == 0)
                    {
                        fprintf(fp_out, "ServerPort=%s\n", ntp_port[0] ? ntp_port : "");
                        continue;
                    }
                    if (strncmp(lines[i], "timezone=", 9) == 0)
                    {
                        fprintf(fp_out, "timezone=%s\n", ntp_timezone[0] ? ntp_timezone : "");
                        continue;
                    }
                }
                else if (strcmp(current_section, "PRINT_PROTOCOL_MSG") == 0)
                {
                    if (strncmp(lines[i], "NetCard=", 8) == 0)
                    {
                        fprintf(fp_out, "NetCard=%s\n", log_netcard[0] ? log_netcard : "");
                        continue;
                    }
                    if (strncmp(lines[i], "RemoteIP=", 9) == 0)
                    {
                        fprintf(fp_out, "RemoteIP=%s\n", log_ip[0] ? log_ip : "");
                        continue;
                    }
                    if (strncmp(lines[i], "StartPortNum=", 13) == 0)
                    {
                        fprintf(fp_out, "StartPortNum=%s\n", log_port[0] ? log_port : "");
                        continue;
                    }
                }

                fputs(lines[i], fp_out);
            }
            fclose(fp_out);
            // 原子替换
            if (rename(tmp_file, BUSLINE_FILE) != 0)
            {
                LOG_ERROR("无法替换BusLine.ini");
                unlink(tmp_file);
            }
            else
            {
                chmod(BUSLINE_FILE, 0666);
            }
        }
        else
        {
            LOG_ERROR("无法创建临时文件");
        }
    }

    FILE *fp = fopen(COMTRADE_FILE, "w");
    if (fp)
    {
        fprintf(fp, "[comtrade_cfg]\n");
        fprintf(fp, "data_type=%s\n", comtrade_type[0] ? comtrade_type : "");
        fclose(fp);
    }

    cJSON_Delete(root);

    system("sync");
    send_success("硬件配置保存成功");
    LOG_INFO("硬件配置保存成功");
    return 0;
}

/*获取设备时间*/
int hw_get_time(void)
{
    char time_str[64];
    get_current_time(time_str, sizeof(time_str));
    send_success(time_str); // 返回 {"success": true, "message": "2026-03-27 15:44:50"}
    LOG_INFO("获取设备时间成功", time_str);
    return 0;
}

/* 设置硬件时钟 */
int hw_set_time(void)
{
    LOG_DEBUG("设置系统时间");
    char *content_length = getenv("CONTENT_LENGTH");
    if (!content_length)
    {
        send_error("请求数据为空");
        return 1;
    }

    int len = atoi(content_length);
    char *buffer = malloc(len + 1);
    if (!buffer)
    {
        send_error("内存分配失败");
        return 1;
    }
    fread(buffer, 1, len, stdin);
    buffer[len] = '\0';

    LOG_INFO("接收到设置时间: %s", buffer);

    // 解析 JSON 获取 time 字段
    cJSON *json = cJSON_Parse(buffer);
    free(buffer);
    if (!json)
    {
        send_error("JSON 解析失败");
        return 1;
    }
    cJSON *time_item = cJSON_GetObjectItem(json, "time");
    if (!cJSON_IsString(time_item))
    {
        send_error("缺少 time 字段或格式错误");
        cJSON_Delete(json);
        return 1;
    }
    const char *time_str = time_item->valuestring;

    // 构建命令，使用 date -s 设置系统时间
    char cmd[256];
    snprintf(cmd, sizeof(cmd), "date -s '%s' 2>&1", time_str);
    FILE *fp = popen(cmd, "r");
    if (!fp)
    {
        send_error("执行 date 命令失败");
        cJSON_Delete(json);
        return 1;
    }
    char result[128] = {0};
    fread(result, 1, sizeof(result) - 1, fp);
    int status = pclose(fp);
    if (status != 0)
    {
        // 命令执行失败
        char err_msg[256];
        snprintf(err_msg, sizeof(err_msg), "设置系统时间失败: %s", result);
        send_error(err_msg);
        cJSON_Delete(json);
        return 1;
    }

    // 将系统时间写入硬件时钟
    fp = popen("hwclock -w -u 2>&1", "r");
    if (!fp)
    {
        send_error("执行 hwclock 命令失败");
        cJSON_Delete(json);
        return 1;
    }
    memset(result, 0, sizeof(result));
    fread(result, 1, sizeof(result) - 1, fp);
    status = pclose(fp);
    if (status != 0)
    {
        char err_msg[256];
        snprintf(err_msg, sizeof(err_msg), "写入硬件时钟失败: %s", result);
        send_error(err_msg);
        cJSON_Delete(json);
        return 1;
    }

    cJSON_Delete(json);
    send_success("系统时间设置成功");
    system("sync");
    return 0;
}

/* 路由处理 */
int handle_request(void)
{
    char *query_string = getenv("QUERY_STRING");
    char *request_method = getenv("REQUEST_METHOD");

    if (!query_string)
    {
        send_error("无效的请求");
        return 1;
    }

    LOG_INFO("收到请求: %s %s", request_method ? request_method : "GET", query_string);

    if (strcmp(query_string, "bus_get_config") == 0)
    {
        return bus_get_config();
    }
    else if (strcmp(query_string, "bus_set_config") == 0)
    {
        return bus_set_config();
    }
    else if (strcmp(query_string, "bus_get_status") == 0)
    {
        return bus_get_status();
    }
    else if (strcmp(query_string, "hw_get_config") == 0)
    {
        return hw_get_config();
    }
    else if (strcmp(query_string, "hw_set_config") == 0)
    {
        return hw_set_config();
    }
    else if (strcmp(query_string, "hw_set_time") == 0)
    {
        return hw_set_time();
    }
    else if (strcmp(query_string, "hw_get_time") == 0)
    {
        return hw_get_time();
    }
    else if (strcmp(query_string, "login") == 0)
    {
        return user_login();
    }
    else if (strcmp(query_string, "change_password") == 0)
    {
        return change_password();
    }
    else if (strcmp(query_string, "download_61850log") == 0)
    {
        return download_61850log();
    }
    else if (strstr(query_string, "upload_firmware") == query_string)
    {
        return upload_firmware();
    }
    /* 新增：状态查询 */
    else if (strstr(query_string, "upgrade_status"))
    {
        return get_upgrade_status();
    }
    else if (strcmp(query_string, "reboot") == 0)
    {
        return system_reboot();
    }
    else if (strcmp(query_string, "get_system_info") == 0)
    {
        return get_system_info();
    }

    send_error("未知的请求");
    return 1;
}

/* 主函数 */
int main(int argc, char *argv[])
{
    /* 初始化zlog */
    if (zlog_init(ZLOG_CONF_FILE))
    {
        fprintf(stderr, "zlog初始化失败\n");
        return 1;
    }

    zlog_cat = zlog_get_category("web");
    if (!zlog_cat)
    {
        fprintf(stderr, "获取zlog类别失败\n");
        zlog_fini();
        return 1;
    }

    LOG_INFO("EMU2000 Web CGI启动");
    printf("EMU2000 Web CGI启动\r\n\r\n");

    /* 初始化配置管理器 */
    if (config_manager_init() != 0)
    {
        LOG_ERROR("配置管理器初始化失败");
        zlog_fini();
        return 1;
    }

    /* FastCGI主循环 */
    while (FCGI_Accept() >= 0)
    {
        handle_request();
    }

    LOG_INFO("EMU2000 Web CGI停止");

    /* 清理 */
    zlog_fini();

    return 0;
}