/**
 *   \file iec61850_macro.h
 *   \brief 宏定义，方便以后进行配置
 */
#ifndef _IEC61850_MACRO_H_
#define _IEC61850_MACRO_H_

#include "config.h"

#ifdef PLATFORM_DESK
#define IEC61850_CONFIG_PATH "/home/mengqp/code/PXR-61850Com/src/61850/cfg/iec61850/"
#define OUTDATA_TYPE_EMU2000_SHM
#define PLATFORM_NAME "DESK"
#elif defined(PLATFORM_T113)
#define IEC61850_CONFIG_PATH "/mynand/config/iec61850/"
#define OUTDATA_TYPE_EMU2000_SHM
#define PLATFORM_NAME "T113"
#else
#define IEC61850_CONFIG_PATH "/mynand/config/iec61850/"
#endif

#include "ied_macro.h"
#include "model_macro.h"
#include "emu2000shm_macro.h"
#include "iec61850_conf_macro.h"
#include "dev_macro.h"

#define LED_61850 (0)
#define LED_61850_BLINK_INTERVAL_MS (500)


#endif /* _IEC61850_MACRO_H_ */
