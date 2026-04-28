/**
 *   \file outval.h
 *   \brief 外部数据值
 */
#ifndef _OUTVAL_H_
#define _OUTVAL_H_

#include <stdbool.h>
#include <stdint.h>
#include "dev_data.h"

/*  */
typedef struct _OUTVAL_EVENT_TIME_T {
    union {
        uint64_t utc_time;
        struct {
            unsigned short year;
            unsigned char month;
            unsigned char day;
            unsigned char hour;
            unsigned char min;
            unsigned char sec;
            unsigned short msec;
        };
    };
} OUTVAL_EVENT_TIME_T;

typedef struct _OUTVAL_EVENT {
    DEV_DATA *pdev; /* 事件名称 */
    char *event_name; /* 事件名称 */
    unsigned char attr; /* 0为变化遥信，1为soe */
    unsigned char bval;
    OUTVAL_EVENT_TIME_T time;
} OUTVAL_EVENT;

/*  */
typedef enum _OUTVAL_EVENT_VAL_TYPE {
    OUTVAL_EVENT_VAL_NULL = 0,
    OUTVAL_EVENT_VAL_FLOAT,
    OUTVAL_EVENT_VAL_INT,
    OUTVAL_EVENT_VAL_UINT,
} OUTVAL_EVENT_VAL_TYPE;

typedef struct _OUTVAL_EVENT_VALS {
    DEV_DATA *pdev; /* 事件名称 */
    char *event_name; /* 事件名称 */
    /* float fval; */
    /* OUTVAL_EVENT_TIME_T time; */
    int num;
    DEV_VAL val[32];
} OUTVAL_EVENT_VALS;

/*  */
typedef struct _OUTVAL_PORT_ADDR {
    char *port; /* 端口号 */
    char *addr; /* 地址 */
    char *guid; /* 标识 */
} OUTVAL_PORT_ADDR;

/*  */
typedef enum _OUTVAL_TYPE {
    OVT_NONE = 0, /*  */
    OVT_BOOL,
    OVT_CHAR,
    OVT_BYTE,
    OVT_SINT,
    OVT_USINT,
    OVT_INT,
    OVT_DBPOS,
    OVT_UINT,
    OVT_DINT,
    OVT_UDINT,
    OVT_I64,
    OVT_U64,
    OVT_FLOAT,
    OVT_DOUBLE,
    OVT_STRING128,
    OVT_TIME,

    OVT_PORT_ADDR,
    OVT_EVENT,
    OVT_EVENT_VALS,
    OVT_DEV_COMMSTATE,
} OUTVAL_TYPE;

typedef enum _OUTVAL_MTYPE {
    OVM_REALTIME = 0x00010000,
    OVM_DEVTYPE,
    OVM_DEVDATA = 0x00020000,
    OVM_RECORD = 0x00030000,

} OUTVAL_MTYPE;

/*  */
typedef struct _OUTVAL_STRUCT {
    int len;
    char *buf;
} OUTVAL_STRUCT;

typedef struct OUT_ACSI_TIMESTAMP
{
	unsigned SecondSinceEpoch:32;
	unsigned FractionOfSecond:24;
	unsigned TimeQuality:8;
}OUT_ACSI_TIMESTAMP;

/*  */
typedef struct _OUTVAL_UPDATE_TIME
{
    int choice; /* 0:null 1: acsi_time 2: utc_time*/

    union {
        uint64_t utc_time;
        OUT_ACSI_TIMESTAMP acsi_time;
    };

}OUTVAL_UPDATE_TIME;


/*  */
typedef struct _OUTVAL {
    char *name;
    int odt;
    DEV_DATA *pdev;
    union {
        int type;
        struct {
            uint16_t ntype;
            uint16_t mtype;
        };
    };
    OUTVAL_UPDATE_TIME t;

    union {
        bool bval;
        char cval;
        uint8_t byval;
        int16_t sval;
        uint16_t wval;
        int32_t ival;
        uint32_t doval;
        int64_t i64val;
        uint64_t u64val;
        float fval;
        double dval;

        OUTVAL_STRUCT struct_val;
    };
} OUTVAL;

/*  */
typedef struct _OUTVAL_LIST {
    int num;
    OUTVAL *vals;
} OUTVAL_LIST;

/**
 *  \brief 获取数据
 *  \param *pdata
 *  \param OUTVAL *
 *  \return ERR_OK 成功
 */
int outval_get(OUTVAL *pval);

int outval_get_guid(DEV_DATA *pdev);
int outval_get_commstate(DEV_DATA *pdev);
int outval_get_event(OUTVAL_EVENT *pevent);
int outval_get_event_vals(OUTVAL_EVENT_VALS *pevent);

#endif /* _OUTVAL_H_ */
