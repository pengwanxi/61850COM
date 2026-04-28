/********************************************************************
 *  自定义变量类型
 ********************************************************************/
#ifndef  _TYPEDEF_H_
#define  _TYPEDEF_H_

#include <unistd.h>
#include <string.h>


#ifdef __cplusplus
extern "C" {
#endif

	typedef short          int16;
	typedef int            int32;
	typedef unsigned char  int8u;
	typedef unsigned short int16u;
	typedef unsigned int   int32u;

	typedef unsigned char  BYTE;
	typedef unsigned short WORD;
	typedef unsigned int   UINT;
	typedef unsigned long  DWORD;
    typedef unsigned long long QWORD;
	typedef int			 BOOL;
	typedef int			 HANDLE;
	typedef long           LONG;
	typedef char*			 LPSTR;
	typedef const char*    LPCSTR;
	typedef void*          LPVOID;

#define TRUE   1
#define FALSE  0
#define ERROR  (-1)

#define HIBYTE(x)	(((x) >> 8) & 0xff)	   /* most signif byte of 2-byte integer */
#define LOBYTE(x)	((x) & 0xff)		   /* least signif byte of 2-byte integer*/
#define HIWORD(x)   (((x) >> 16) & 0xffff) /* most signif word of 2-word integer */
#define LOWORD(x)   ((x) & 0xffff) 		   /* least signif byte of 2-word integer*/

#define MAKEWORD(l, h)	(((h) << 8) | (l))
#define MAKELONG(l, h)	(((h) << 16) | (l))

#ifndef max
	//#define max(x, y)	(((x) < (y)) ? (y) : (x))
#endif

#ifndef min
	//#define min(x, y)	(((x) < (y)) ? (x) : (y))
#endif

#ifdef __cplusplus
}
#endif

#pragma pack(1)

typedef struct _tagTIMEDATA
{
	unsigned int MiSec:10;
	unsigned int Second:6;
	unsigned int Minute:6;
	unsigned int Hour:5;
	unsigned int Day:5;
	unsigned int Month:4;
	unsigned int Year:12;
} TIMEDATA;

typedef struct _tagREALTIME {
	unsigned short wYear;
	unsigned short wMonth;
	unsigned short wDayOfWeek;
	unsigned short wDay;
	unsigned short wHour;
	unsigned short wMinute;
	unsigned short wSecond;
	unsigned short wMilliSec;
} REALTIME;

#pragma pack()

#endif   /*_TYPEDEF_H*/
/*========================== 本文件结束 ==============================*/
