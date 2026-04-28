/**
 *   \file emu2000ini.h
 *   \brief 通过读emu2000的ini文件知道配置
 */
#ifndef _EMU2000INI_H_
#define _EMU2000INI_H_

#include "iec61850_conf.h"
/**
 *  \brief ini 文件解析
 *  \param IEC61850_CONF  配置结构体
 *  \return ERR_OK 成功
 */
int emu2000_config_ini_parse(IEC61850_CONF *pcfg);

#endif /* _EMU2000INI_H_ */
