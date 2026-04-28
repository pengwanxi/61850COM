#include "ini.h"
#include "err_def.h"
#include "emu2000ini.h"
#include "emu2000ini_busline.h"

int emu2000_config_ini_parse(IEC61850_CONF *pcfg)
{
    return emu2000_config_busline_ini_parse(pcfg);
}
