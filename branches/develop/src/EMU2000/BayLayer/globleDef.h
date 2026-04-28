#ifndef  _GLOBLEDEF_H
#define _GLOBLEDEF_H
 
//#define Debug
#define Release

#ifdef Debug
#define SYSTEM_PATH		"/mnt"
#else
#define SYSTEM_PATH	   "/mynand"
#endif

//通讯协议类型
#define MODBUS_RTU	 1
#define	IEC104_SLAVE	2
#define IEC103_RTU   3
#define	DDB_RTU			/* 双机 */


#endif


