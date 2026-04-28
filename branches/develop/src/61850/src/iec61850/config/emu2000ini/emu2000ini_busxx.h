/**
 *   \file emu2000ini_busxx.h
 *   \brief 读协议里面的busxx.h
 */
#ifndef _EMU2000INI_BUSXX_H_
#define _EMU2000INI_BUSXX_H_

#include "iec61850_conf.h"

/*  */
typedef enum _EMU2000_DEVTYPE
{
    EMU2000_DEVTYPE_NULL = 0,
    EMU2000_DEVTYPE_PXR20,
    EMU2000_DEVTYPE_PXR25,
    EMU2000_DEVTYPE_SIZE,
}EMU2000_DEVTYPE;


/*  */
typedef struct _EMU2000_DEVTYPE_DATA
{
    int num;
    int mask;
    int devtype[32];

}EMU2000_DEVTYPE_DATA;


int emu2000_config_busxx_ini_parse(IEC61850_CONF *pcfg, char *name, int idx);

#endif /* _EMU2000INI_BUSXX_H_ */
