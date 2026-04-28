#ifndef __CONFIG_MANAGER_H__
#define __CONFIG_MANAGER_H__

#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <ctype.h>
#include <unistd.h>
#include <sys/stat.h>
#include <errno.h>

#define MAX_COMM_ROWS 50
#define MAX_DEVICES_PER_ROW 100
#define MAX_DEVICE_NAME_LEN 64
#define MAX_ADDRESS_LEN 32
#define MAX_EXTRA_PARAMS_LEN 128

typedef enum {
    BUS_TYPE_UNKNOWN = 0,
    BUS_TYPE_ETH0,
    BUS_TYPE_RS485
} bus_type_t;

typedef enum {
    TEMPLATE_UNKNOWN = 0,
    TEMPLATE_EATONPXR20_TCP,
    TEMPLATE_EATONPXR20_RTU,
    TEMPLATE_EATONPXR25_TCP,
    TEMPLATE_EATONPXR25_RTU
} template_type_t;

typedef enum {
    PROTOCOL_MODBUS_MASTER = 1
} protocol_type_t;

#define MODULE_PXR_20 1
#define MODULE_PXR_25 2

typedef struct {
    int id;
    int order;
    char address[MAX_ADDRESS_LEN];
    char name[MAX_DEVICE_NAME_LEN];
    template_type_t template_type;
    protocol_type_t protocol;
    int module;
} device_t;

typedef struct {
    int id;
    bus_type_t type;
    char type_name[64];
    char device_name[MAX_DEVICE_NAME_LEN];
    char ip[16];        // 网口IP地址
    int port;
    int internal;
    char extra_params[MAX_EXTRA_PARAMS_LEN];
    int device_count;
    device_t devices[MAX_DEVICES_PER_ROW];
} comm_row_t;

typedef struct {
    int row_count;
    comm_row_t rows[MAX_COMM_ROWS];
} bus_config_t;

int config_manager_init(void);
int load_bus_config(bus_config_t *config);
int save_bus_config(const bus_config_t *config);
bus_type_t parse_bus_type(const char *port_value);
const char* bus_type_to_string(bus_type_t type);
const char* bus_type_to_display_name(bus_type_t type);
template_type_t parse_template_type(const char *template_str);
const char* template_type_to_string(template_type_t tmpl);
const char* protocol_type_to_string(protocol_type_t protocol);
const char* protocol_type_to_display_name(protocol_type_t protocol);
protocol_type_t parse_protocol_type(const char *protocol_str);
int template_to_module(template_type_t tmpl);

#endif
