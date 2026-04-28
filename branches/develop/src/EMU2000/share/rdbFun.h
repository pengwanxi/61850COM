/********************************************************************
 *  数据库访问函数
 ********************************************************************/
#ifndef  _RDBFUN_H_
#define  _RDBFUN_H_

#include "rdbDef.h"
//#include "stdio.h"
/*******************************************************************/
#ifdef	__cplusplus
extern "C" {
#endif	/* __cplusplus */

	int  Open_SHM_DBase();
	void Close_SHM_DBase();
	int  Create_SHM_DBase(char* szPath, int nExtLen);
	int  Check_SHM_DBase();
	BOOL EnableDebug(BOOL bEnable);
	void ShowRTDBInfo();

	char *GetWorkPath();
	char *Get_RTDB_Space();
	int   Get_RTDB_Extend(int *addr);
	const SYSINFO* Get_RTDB_SysInfo();
	const STNPARAM* Get_RTDB_Station(WORD wStnNum);
	const ANALOGITEM*  Get_RTDB_Analog(WORD wStn, WORD wPnt);
	const DIGITALITEM* Get_RTDB_Digital(WORD wStn, WORD wPnt);
	const RELAYITEM* Get_RTDB_Relay(WORD wStn, WORD wPnt);
	const PULSEITEM* Get_RTDB_Pulse(WORD wStn, WORD wPnt);
	const DZITEM* Get_RTDB_DZ(WORD wStn, WORD wPnt);
	const SOEITEM* Read_RTDB_SOE(int iPos);
	const AIEITEM* Read_RTDB_AIE(int iPos);

	int Write_RTDB_Data(unsigned char *pBuf, int nLen);
	int Read_RTDB_Data(unsigned char *pBuf, int nLen);

	int LoginMessageBus(char *szProcName);
	int ExitMessageBus(int nProcKey);
	int MessageSend(MSGITEM *pMessage, char *pDst);
	int MessageRecv(int nProcKey, MSGITEM *pMessage, int nSync);
	int MessageSubscribe(int nProcKey, unsigned int dwOption);
	int MessageUnSubscribe(int nProcKey, unsigned int dwOption);
	void ltrim(char *s);
	void rtrim(char *s);
	int  GetTextLine(FILE* fd, char* lpszBuf, int nSize);
	int stricmp(const char *s, const char *t);
	void LogPromptText(const char *fmt, ...);
	int GetPntSum(BYTE byType);
	int GetTransNum(BYTE byType, WORD wStn, WORD wPnt);
	class CSerialPort ;
	class CBasePort ;
	class CTcpListen ;
	class CUdpPort ;
	class CProfile;
	class CTcpPortServer ;
	class CProtocol;
	class CMethod ;
	class CSemObj ;
	class CRtuBase ;
	class CTcpClient ;
	class CTcpClientShort ;
	class CSocketFtp;
	class CLoraPort;
	class C61850TransferFile;
#ifdef	__cplusplus
}
#endif	/* __cplusplus */
/*******************************************************************/
#endif   /*_RDBFUN_H*/
