/*
 * =====================================================================================
 *
 *       Filename:  global.h
 *
 *
 *    Description:  增加版本号
 *        Version:  1.1
 *        Created:   2015年10月12日 08时53分03秒
 *
 *    Description:  定义全局函数
 *        Version:  1.0
 *        Created:  2014年09月11日 11时18分52秒
 *
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  YOUR NAME (),
 *   Organization:
 *
 * =====================================================================================
 */

#ifndef  GLOBAL_INC
#define  GLOBAL_INC


#define		EMU2000_VERSION			0x01040112					/* EMU2000版本号 */
#ifdef  __cplusplus
extern "C" {
#endif
#include <signal.h>
#include <time.h>

#include "typedef.h"
#include "gDataType.h"
typedef void (*LPSIGPROC)(int, siginfo_t*, void*);

int already_running(const char *filename);
struct tm  SetStructTm ( TIMEDATA srcTime, struct tm &t, long &lTime );
void GetOwnStructTm ( long lTime, struct tm *t );
void ltrim(char *s);
void rtrim(char *s);
void OutPromptText(char *lpszText);
void LogPromptText(const char *fmt, ...);
void OutMessageText(char *szSrc, unsigned char *pData, int nLen);
int SignalHook(int iSigNo, LPSIGPROC func);
    void init_daemon(void);
    bool GlobalCopyByEndian( unsigned char *dest, unsigned char *src, unsigned int num );
    bool IsBigEndian(void);
#ifdef  __cplusplus
}
#endif

/*lel*/
#if 1
#define MAX_BUFFER				    1024000
#else
#define MAX_BUFFER				    1024
#endif
/*end*/
#define MAX_DEBUG_BUFFER	3 * 1024 + 100

//ͨѶ���Ͷ���
#define COM_PAUSE  0
#define COMRS232		1
#define COMRS485		2
#define COMRS422		3
#define SOCKETTCP	4
#define TCP_CLIENT	5
#define TCP_CLIENT_SHORT	6
#define  CAN_NET        7
#define  LORA_WIRELESS			8

#define PASUE			 "PAUSE"
#define COMRS_232	"COMRS232_"
#define COMRS_485	"COMRS485_"
#define COMRS_422	"COMRS422_"
#define LAN_TCP			"TCP_"
#define LAN_TCP_CLIENT	"TCPCLIENT_"
#define LAN_TCP_CLIENT_SHORT	"TCPCLIENT-SHORT_"
#define COM_CAN_NET           "can_"
#define COM_LORA		"LORA_"

#define GPIO_DEV_NAME "/dev/gpio_drv"
#define CMD_SET_UT1_RS485	 11
#define CMD_SET_UT1_NON_RS485	 12 //���ô���1ΪRS232�ӿ�
#define CMD_SET_UT2_RS485	 13
#define CMD_SET_UT2_NON_RS485	 14 //���ô���2ΪRS232�ӿ�
#define CMD_SET_UT3_RS485	 15
#define CMD_SET_UT3_NON_RS485	 16 //���ô���3ΪRS485�ӿ�
#define CMD_SET_UT4_RS485	 17
#define CMD_SET_UT4_NON_RS485	 18 //���ô���4ΪRS485�ӿ�
#define CMD_GET_UT_STATUS	 19

void SetSerialConfigMode(  int byMode ) ;

#endif   /* ----- #ifndef GLOBAL_INC  ----- */
