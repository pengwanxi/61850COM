/**
 *   \file iec61850_conf_macro.h
 *   \brief iec61850配置定义
 */
#ifndef _IEC61850_CONF_MACRO_H_
#define _IEC61850_CONF_MACRO_H_

#define CONFIG_PATH  IEC61850_CONFIG_PATH

#define IEC61850_CFG_DEV_MAX (32)
#define IEC61850_CFG_TEMPLATE_MAX (32)
#define IEC61850_CFG_TEMPLATE_DATASET_MAX (256)
#define IEC61850_CFG_TEMPLATE_EVENT_MAX (128)

#define DEV_TYPE_METER "meter"
#define DEV_TYPE_IED "ied"

#define DEV_TYPE_MATCH_METER(type) (0 == strcmp(type, DEV_TYPE_METER))
#define DEV_TYPE_MATCH_IED(type) (0 == strcmp(type, DEV_TYPE_IED))

#define TEMPLATE_PROTO_LOCAL_IEC61850 "iec61850"
#define TEMPLATE_PROTO_EMU2000_SHM "emu2000shm"
#define TEMPLATE_PROTO_SCU_JSON "scujson"

#define TEMPLATE_PROTO_MATCH_IEC61850(proto)                                   \
    (0 == strcmp(proto, TEMPLATE_PROTO_LOCAL_IEC61850))

#define TEMPLATE_PROTO_MATCH_EMU2000_SHM(proto)                                \
    (0 == strcmp(proto, TEMPLATE_PROTO_EMU2000_SHM))

#define TEMPLATE_PROTO_MATCH_SCU_JSON(proto)                                   \
    (0 == strcmp(proto, TEMPLATE_PROTO_SCU_JSON))

#define TEMPLATE_PX20 "px20"
#define TEMPLATE_PX25 "px25"
#define TEMPLATE_IED "ied"

#define TEMPLATE_MATCH_PX20(model) (0 == strcmp(model, TEMPLATE_PX20))
#define TEMPLATE_MATCH_PX25(model) (0 == strcmp(model, TEMPLATE_PX25))
#define TEMPLATE_MATCH_IED(model) (0 == strcmp(model, TEMPLATE_IED))


#endif /* _IEC61850_CONF_MACRO_H_ */
