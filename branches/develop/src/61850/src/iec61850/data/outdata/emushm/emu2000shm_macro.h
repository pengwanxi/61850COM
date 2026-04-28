/**
 *   \file emu2000shm_macro.h
 *   \brief 宏定义
 */
#ifndef _EMU2000SHM_MACRO_H_
#define _EMU2000SHM_MACRO_H_

#define EMU2000_SHMDBKEY 20193568

#define EMU2000_DATA_TYPE_YC ("YC")
#define EMU2000_DATA_TYPE_YX ("YX")
#define EMU2000_DATA_TYPE_YM ("YM")
#define EMU2000_DATA_TYPE_YM_TIME ("YM_TIME")
#define EMU2000_DATA_TYPE_YM_ALM ("YM_ALM")
#define EMU2000_DATA_TYPE_YK ("YK")
#define EMU2000_DATA_TYPE_DZ ("DZ")

#define EMU2000_DATA_TYPE_YXPOS ("YXPOS")


#define EMU2000_DATA_TYPE_LDNAME ("LDNAME")
#define EMU2000_DATA_TYPE_SWREV ("SWREV")
#define EMU2000_DATA_TYPE_PLTD ("PLTD")
#define EMU2000_DATA_TYPE_CONFREV ("CONFREV")
#define EMU2000_DATA_TYPE_LDNS ("YM_LDNS")

#define EMU2000_CONFIG_PATH "/mynand/config"
#define EMU2000_BUSLINE_INI_NAME "/mynand/config/BusLine.ini"
#define EMU2000_INI_MATCH(s, n) strcmp(section, s) == 0 && strcmp(name, n) == 0


#endif /* _EMU2000SHM_MACRO_H_ */
