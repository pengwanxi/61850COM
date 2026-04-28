/**
 *   \file dev_val.h
 *   \brief 装置数据值
 */
#ifndef _DEV_VAL_H_
#define _DEV_VAL_H_

#include <stdbool.h>
#include <stdint.h>

/*  */
typedef struct _DEV_VAL_STRING {
    char *val;
} DEV_VAL_STRING;

/*  */
typedef struct _DEV_VAL_EVENT {
    char ref_name[256];
    char ref_time[256];

    int ref_q_valid;
    char ref_q[256];

    unsigned char state;
    uint64_t utc_time;
} DEV_VAL_EVENT;

/* 装置数据类型 */
typedef enum _DEV_VAL_TYPE {
    DVT_INVALID = 0, /* 无效类型 */

    DVT_BOOLEAN, /* 布尔型 */
    DVT_CHAR,
    DVT_BYTE, /* 字节型 */
    DVT_SHORT, /* 短整型 */
    DVT_WORD, /* 无符号短整型 */
    DVT_INT, /* 整型 */
    DVT_DWORD, /* 无符号整型 */
    DVT_FLOAT, /* 浮点型 */
    DVT_DOUBLE, /* 浮点型 */
    DVT_STRING, /* 字符串型 */
    DVT_DBPOS, /* Dbpos */

    DVT_UTCTIME, /* 时间 */
    DVT_EVENT, /* 事件类型 */
    DVT_REFNAME, /* 事件类型 */

    DVT_COMMSTATE_ERROR, /* 通信异常 */
} DEV_VAL_TYPE;

typedef struct DEVAL_ACSI_TIMESTAMP
{
	unsigned SecondSinceEpoch:32;
	unsigned FractionOfSecond:24;
	unsigned TimeQuality:8;
}DEVAL_ACSI_TIMESTAMP;


typedef struct _DEV_VAL_UPDATE_TIME
{
    int choice; /* 0:null 1: acsi_time 2: utc_time*/

    union {
        uint64_t utc_time;
        DEVAL_ACSI_TIMESTAMP acsi_time;
    };

}DEV_VAL_UPDATE_TIME;


/* 装置数据 */
typedef struct _DEV_VAL {
    char *name;
    char *refname;
    DEV_VAL_TYPE type;

    char *ref_time;
    DEV_VAL_UPDATE_TIME t;

    char *ref_q;

    union {
        bool bval;
        char cval;
        uint8_t byval;
        int16_t sval;
        uint16_t wval;
        int32_t ival;
        uint32_t doval;
        float fval;
        double dval;
        uint64_t utc_time;

        DEV_VAL_STRING str;
        DEV_VAL_EVENT *event;
    };

} DEV_VAL;

/*  */
typedef enum _DEV_VAL_LIST_TYPE {
    DVLT_DEFAULT = 0,
    DVLT_TEMPLATE,

} DEV_VAL_LIST_TYPE;

/*  */
typedef struct _DEV_VAL_LIST {
    int num;
    int type;
    DEV_VAL *vals;
} DEV_VAL_LIST;

#endif /* _DEV_VAL_H_ */
