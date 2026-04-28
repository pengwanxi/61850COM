#include "main.h"
#include "BusManger.h"
#include "CPublicMethod.h"
#include "../share/global.h"
#include "../share/md5.h"
#include <iostream>
#include <unistd.h>
#include <sys/reboot.h>

#include <sys/types.h>
#include <sys/param.h>
#include <sys/time.h>
#include <sys/ioctl.h>
#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/mman.h>
#include <paths.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <err.h>
#include <sys/ioctl.h>

// #include <sys/sysctl.h>
#include <time.h>
#include <sys/time.h>

#include "../share/Clog.h"
#include "../Protocol/DevCheck/CSocketDevCheck.cpp"

#ifdef ARM
#include "../share/can_port/CanNet.h"
#endif

#ifdef LORA
#include "../share/lora/LoraPort.h"
#endif

using namespace std;
#define GPIO_DEV_NAME "/dev/gpio_drv"
#define CMD_SET_UT1_RS485 11
#define CMD_SET_UT1_NON_RS485 12
#define CMD_SET_UT2_RS485 13
#define CMD_SET_UT2_NON_RS485 14
#define CMD_SET_UT3_RS485 15
#define CMD_SET_UT3_NON_RS485 16
#define CMD_SET_UT4_RS485 17
#define CMD_SET_UT4_NON_RS485 18
#define CMD_GET_UT_STATUS 19

#define CMD_SET_LEDRUN_ON 21	// LED运行灯 开
#define CMD_SET_LEDRUN_OFF 22	// LED运行灯 关
#define CMD_SET_LEDALARM_ON 23	// 报警灯 开
#define CMD_SET_LEDALARM_OFF 24 // 报警灯 关
#define CMD_SET_LEDRES3_ON 25	// 问问
#define CMD_SET_LEDRES3_OFF 26	// 问问

#define CMD_GET_NET_MODE 31

extern "C" void GetCurrentTime(REALTIME *pRealTime);

void OutSerialSum(int iSerialSum);
Clog m_log;

void getCurrentTimeStr(char *time_buf)
{
	REALTIME curTime;
	GetCurrentTime(&curTime);

	sprintf(time_buf, "%04d-%02d-%02d %02d:%02d:%02d",
			curTime.wYear, curTime.wMonth, curTime.wDay,
			curTime.wHour, curTime.wMinute, curTime.wSecond);
}

// 管理总线对象 析构时候会释放所有端口
CBusManger m_busManager;
// 用来打印报文到网页
CTcpPortServer g_printMsgTcpPort;
CMsg g_printMsgQueue;
DWORD g_BusNo = 0xFFFF;
string g_szUniqueCode("");

// LED灯开始
#define LED_BEGIN 1
#define LED_END 2

void SetSerialMode()
{ /*{{{*/
	int gpiofd;
	/*open gpio drv*/
	gpiofd = open(GPIO_DEV_NAME, O_RDWR);
	if (gpiofd < 0)
		return;
	ioctl(gpiofd, CMD_SET_UT1_RS485, 0);
	ioctl(gpiofd, CMD_SET_UT2_RS485, 0);
	ioctl(gpiofd, CMD_SET_UT3_RS485, 0);
	ioctl(gpiofd, CMD_SET_UT4_RS485, 0);
	close(gpiofd);
} /*}}}*/
/*****************************************************************************/
BOOL g_bDebugApp = TRUE;
BOOL g_bAppRun = TRUE; // 进程运行

#define THREAD_EXIT_TIME 50 /* 循环次数  */

#define MSGSET_OPTION ((1 << MSGSET_CTRL_DATA) | (1 << MSGSET_DEVS_COMM))
int g_nBusKeyID = -1; // 消息总线key
// CPortArray g_PortArray;  //通讯对象
char g_szAppName[24] = {"modbus"};
extern CRTDBObj *g_pRTDBObj; /*应用程序主对象*/

/*****************************************************************************/
#ifdef __cplusplus
extern "C"
{	   /*{{{*/
#endif /* __cplusplus */

	int already_running(const char *filename);

	// 删除字符串左边空格及\t
	void ltrim(char *s);

	// 删除字符串右边空格及'\t','\r','\n'
	void rtrim(char *s);
	void OutPromptText(char *lpszText);
	void LogPromptText(const char *fmt, ...);
	void OutMessageText(char *szSrc, unsigned char *pData, int nLen);

	/*
	 * ===  FUNCTION  ======================================================================
	 *         Name:  OutBusDebug
	 *  Description:  flag: 0 二进制发送 1二进制接收 2 ascii发送 3 ascii接受
	 *				注：最多打印MAX_DEBUG_LEN个字节
	 * =====================================================================================
	 */

#define MAX_DEBUG_LEN 1024 /*打印的最大缓冲区  */
	void OutBusDebug(BYTE byBusNo, BYTE *buf, int len, int flag)
	{ /*{{{*/
		int nCount = m_busManager.m_sbus.size();
		PBUSMANAGER pBus;
		char szBuff[3 * MAX_DEBUG_LEN + 100];
		int i, k = 0;
		memset(szBuff, 0, 3 * MAX_DEBUG_LEN + 100);

		if (len > MAX_DEBUG_LEN)
			len = MAX_DEBUG_LEN;
		if (nCount <= 0)
		{
			return;
		}
		for (i = 0; i < nCount; i++)
		{
			pBus = m_busManager.m_sbus[i];
			if (!pBus->m_Protocol || !pBus->m_pMethod || !pBus->m_Port)
				continue;

			if (byBusNo == pBus->byBusNo)
			{
				break;
			}
		}
		if (i >= nCount)
		{
			printf("OutBusDebug not find busline=%d\n", byBusNo);
		}
		if (flag == 0x01)
			k = sprintf(szBuff, "RX:(BUS%d len=%d)", byBusNo + 1, len);
		else if (flag == 0)
			k = sprintf(szBuff, "TX:(BUS%d len=%d)", byBusNo + 1, len);
		else
			k = sprintf(szBuff, "                 ");
		if ((flag & 0x2) != 0)
		{
			memcpy(&szBuff[k], buf, len);
			k += len;
		}
		else
		{
			for (int i = 0; i < len; i++)
			{
				k += sprintf(&szBuff[k], "%02X ", buf[i]);
			}
		}
		k += sprintf(&szBuff[k], "\n");

		// 打开网页查看报文的同时 也向配置好的打印IP和端口发送udp报文

		pBus->m_Debug.SendDebugMsg((BYTE *)szBuff, k);

		WORD wBusNo = g_BusNo - 1;

		if (wBusNo != byBusNo)
			return;

		////打开网页查看报文的同时 也向配置好的打印IP和端口发送udp报文
		// printf("*********%d %s*****byBusNo=%d*******\n", __LINE__, __FILE__, byBusNo);
		// m_busManager.m_Debug.SendDebugMsg((BYTE *)szBuff, k);

		if (g_printMsgTcpPort.IsPortValid())
		{
			printf("*********%d %s*****byBusNo=%d*******\n", __LINE__, __FILE__, byBusNo);
			int n = g_printMsgTcpPort.WritePort((BYTE *)szBuff, k);
		}
	} /*}}}*/

	void SignHandler(int signum, siginfo_t *pInfo, void *pReserved)
	{
		g_bAppRun = FALSE;
	}

#ifdef __cplusplus
} /*}}}*/
#endif /* __cplusplus */

/*****************************************************************************/
#define CONFIG_TYPE_PARAM 0x0001
#define CONFIG_APP_ALIAS 0x1001
#define CONFIG_SCAN_CYCLE 0x1002
#define CONFIG_DEBUG_MODE 0x1003
#define CONFIG_PORT_PARAM 0x2001
#define CONFIG_UNIT_PARAM 0x3001

int ParseConfigItem(char *strItem, WORD *pwNum)
{ /*{{{*/
	char strType[32];
	int i, nLen;

	if (strstr(strItem, "alias"))
		return CONFIG_APP_ALIAS;
	if (strstr(strItem, "debug"))
		return CONFIG_DEBUG_MODE;
	if (strstr(strItem, "cycle"))
		return CONFIG_SCAN_CYCLE;

	i = 0;
	nLen = strlen(strItem);
	while (!isdigit(strItem[i]) && i < (int)sizeof(strType))
	{
		strType[i] = toupper(strItem[i]);
		if (++i >= nLen)
			break;
	}
	strType[i] = '\0';
	if (i >= nLen)
		*pwNum = 0;
	else
		*pwNum = (WORD)atoi(&strItem[i]);
	if (strcmp(strType, "TYPE") == 0)
		return CONFIG_PORT_PARAM;
	if (strcmp(strType, "PORT") == 0)
		return CONFIG_PORT_PARAM;
	if (strcmp(strType, "UNIT") == 0)
		return CONFIG_UNIT_PARAM;
	return -1;
} /*}}}*/

typedef void (*LPSIGPROC)(int, siginfo_t *, void *);

int SignalHook(int iSigNo, LPSIGPROC func)
{ /*{{{*/
	struct sigaction act, oact;

	act.sa_sigaction = func;
	sigemptyset(&act.sa_mask);
	act.sa_flags = SA_SIGINFO | SA_RESTART;

	if (sigaction(iSigNo, &act, &oact) < 0)
		return -1;
	return 0;
} /*}}}*/

/*
 * ===  FUNCTION  ======================================================================
 *         Name:  ReadPrintPara
 *  Description:  获取打印报文的属性  网卡（eth x）远程ip 起始端口
 * =====================================================================================
 */

#define PRINT_MINI_PORTNUM 20000 /*起始打印端口的最小值 */
#define PRINT_MAX_PORTNUM 60000	 /*起始打印端口的最小值 */
static BOOL ReadPrintPara(CProfile *pProfile, char *pNetCard, char *pRemoteIp, DWORD &dwPortNum)
{ /*{{{*/
	char sSect[] = "PRINT_PROTOCOL_MSG";
	char sNetCard[] = "NetCard";
	char sRemoteIp[] = "RemoteIP";
	char sPort[] = "StartPortNum";
	// int iRtn = -1;
	if (pProfile == NULL)
	{
		printf("ReadPrintPara error\n");
		return FALSE;
	}

	// iRtn = pProfile->GetProfileString( sSect , sNetCard , "NULL" , pNetCard , 5 ) ;
	pProfile->GetProfileString(sSect, sNetCard, (char *)"NULL", pNetCard, 5);
	if (strncmp(pNetCard, "NULL", 4) == 0)
	{
		printf("ReadPrintPara NetCard error!!! default set eth0\n");
		strcpy(pNetCard, "eth0");
	}

	// iRtn = pProfile->GetProfileString( sSect , sRemoteIp , "NULL" , pRemoteIp , 16 ) ;
	pProfile->GetProfileString(sSect, sRemoteIp, (char *)"NULL", pRemoteIp, 16);
	if (strncmp(pRemoteIp, "NULL", 4) == 0)
	{
		printf("ReadPrintPara RemoteIp err default set 192.168.1.128\n");
		strcpy(pRemoteIp, "192.168.1.128");
	}

	dwPortNum = (DWORD)pProfile->GetProfileInt(sSect, sPort, 0);
	if (dwPortNum < PRINT_MINI_PORTNUM || dwPortNum > PRINT_MAX_PORTNUM)
	{
		printf("StartPortNum err !!! StartPortNum=%ld(>%d or <%d) default set %d\n",
			   dwPortNum, PRINT_MAX_PORTNUM, PRINT_MINI_PORTNUM, PRINT_MINI_PORTNUM);
		dwPortNum = PRINT_MINI_PORTNUM;
	}

	printf("ReadPrintPara NetCard=%s RemoteIp=%s StartPortNum=%ld\n", pNetCard, pRemoteIp, dwPortNum);

	return TRUE;
} /* -----  end of static function ReadPrintPara  ----- */ /*}}}*/

/*
 * ===  FUNCTION  ======================================================================
 *         Name:  CreateSocketDevCheck
 *  Description:
 *		  Param:
 *		 Return:
 * =====================================================================================
 */
static void CreateSocketDevCheck(CSocketDevCheck **pSocketDevCheck)
{ /*{{{*/

	*pSocketDevCheck = new CSocketDevCheck();
	if (NULL == (*pSocketDevCheck))
	{
		printf("CreateSocketFtp failed\n");
		return;
	}
	printf("*********%d %s************\n", __LINE__, __FILE__);
	if (!(*pSocketDevCheck)->Init())
	{
		printf("CreateSocketDevCheck Init failed\n");
		return;
	}

	return;
}

BOOL ReadPortPara(INITBUS &bus)
{ /*{{{*/
	char sBusLine[] = BUS_PATH;
	CProfile Profile(sBusLine);

	/*设置本地相应网络参数*/
	if (!SetLocalNetPara(Profile))
		return FALSE;

	if (!ReadBusPara(Profile, bus))
		return FALSE;

	if (!ReadFtpServerPara(Profile))
		cout << "Ftp fault Recorder does't steup \n";

	if (!ReadNtpServerPara(Profile))
		cout << "NtpServer read error \n";

	return TRUE;
} /*}}}*/

BOOL ReadNtpServerPara(CProfile &profile)
{
	if (!profile.IsValid())
		return FALSE;

	char szSection[] = "NtpServer";
	char szNetMask[] = "NetMask";
	char szIPField[] = "IPField";
	char szDefault[] = "NULL";
	UNUSED(szDefault);
	char szNetMask_val[100] = {0};
	char szIPField_val[100] = {0};
	profile.GetProfileString(szSection, szNetMask, szDefault, szNetMask_val, sizeof(szNetMask_val));
	if (strcmp(szNetMask_val, szDefault) == 0)
	{
		return FALSE;
	}

	profile.GetProfileString(szSection, szIPField, szDefault, szIPField_val, sizeof(szIPField_val));
	if (strcmp(szIPField_val, szDefault) == 0)
	{
		return FALSE;
	}

	UpdateNtpServerCfg(szNetMask_val, szIPField_val);
	return TRUE;
}

BOOL UpdateNtpServerCfg(char *szNetMask_val, char *szIPField_val)
{
	char szFilePath[150] = {"/etc/ntp.conf"};
	if (access(szFilePath, F_OK) == -1)
	{
		printPromptInfo();
		cout << szFilePath << " : " << "does't exists " << endl;
		return FALSE;
	}

	char buf[200] = {0};
	vector<string> strVec;
	FILE *pFile = fopen(szFilePath, "r+");
	while (fgets(buf, sizeof(buf), pFile))
	{
		string str(buf);
		if (str.find("restrict") != string::npos && str.find("mask") != string::npos && str.find("nomodify") != string::npos && str.find("notrap") != string::npos)
		{
			sprintf(buf, "restrict %s mask %s nomodify notrap", szIPField_val, szNetMask_val);
			cout << "have modified" << ":" << buf << endl;
		}

		strVec.push_back(buf);
	}
	fclose(pFile);

	pFile = fopen(szFilePath, "w");
	for (size_t i = 0; i < strVec.size(); i++)
	{
		cout << strVec.at(i) << endl;
		fwrite((void *)strVec.at(i).c_str(), strlen(strVec.at(i).c_str()), 1, pFile);
	}

	fclose(pFile);

	return TRUE;
}

void printPromptInfo()
{
	cout << endl
		 << "file :" << __FILE__ << " line : " << __LINE__ << " function: " << __FUNCTION__ << endl;
}

BOOL ReadFtpServerPara(CProfile &profile)
{
	if (!profile.IsValid())
		return FALSE;

	char szSection[] = "FaultRecorder";
	char szPortKey[] = "Port";
	char szDefault[] = "NULL";
	char szType[] = "Type";
	UNUSED(szDefault);
	int uPort = 0;
	uPort = profile.GetProfileInt(szSection, szPortKey, -1);
	if (uPort == -1 || uPort <= 0)
	{
		printf("fault recorder hasn't config port error");
		return FALSE;
	}

	char szTypeRet[100] = {0};
	profile.GetProfileString(szSection, szType, "NULL", szTypeRet, sizeof(szTypeRet));
	if (strcmp(szTypeRet, "NULL") == 0)
	{
		printf("fault recorder hasn't config type error");
		return FALSE;
	}

	m_busManager.setFaultRecorderCfg(uPort, szTypeRet);
	return TRUE;
}

BOOL ReadBusPara(CProfile &Profile, INITBUS &bus)
{
	BYTE byLineNum = 0;

	char NetCard[5];
	char RemoteIp[16];
	DWORD StartPortNum;

	if (!ReadPrintPara(&Profile, NetCard, RemoteIp, StartPortNum))
		return FALSE;

	byLineNum = (BYTE)Profile.GetProfileInt((char *)"LINE-NUM", (char *)"NUM", 0);
	if (byLineNum == 0 || byLineNum > MAX_LINE)
	{
		printf("Line Number exceed MaxLine : Current Line Number = %d  \n ", byLineNum);
		return FALSE;
	}

	for (BYTE i = 0; i < byLineNum; i++)
	{
		char sPort[] = "port";
		char sPara[] = "para";
		char sInterval[] = "internal";
		char sSect[] = "PORT";
		int iInterval = 0;
		int sizebuff = 0;
		char sbuffer[200] = {0};
		sizebuff = sizeof(sbuffer);
		char sDllPath[200];
		BOOL bPause = FALSE;
		memset(sDllPath, 0, sizeof(sDllPath));
		char sTemp[200];
		memset(sTemp, 0, sizeof(sTemp));

		memset(sbuffer, 0, sizebuff);

		// 获取通讯口类型
		sprintf(sTemp, "%s%02d", sPort, i + 1);
		Profile.GetProfileString(sSect, sTemp, (char *)"NULL", sbuffer, sizebuff);
		if (strcmp("NULL", sbuffer) == 0)
		{
			printf("Bus Config Error OK. \n");
			return FALSE;
		}
		else if (strcmp(PASUE, sbuffer) == 0)
		{
			printf("Bus%d is PASUE  \n ", i + 1);
			bPause = TRUE;
		}

		if (!bPause)
		{
			// 获取协议类型
			sprintf(sTemp, "%s%02d", sPara, i + 1);
			Profile.GetProfileString(sSect, sTemp, (char *)"NULL", sDllPath, sizeof(sDllPath));
			if (strcmp("NULL", sDllPath) == 0)
			{
				printf("Bus Config Error OK. \n");
				return FALSE;
			}

			// 获取通讯间隔
			sprintf(sTemp, "%s%02d", sInterval, i + 1);
			iInterval = GetBusProtoInterval(Profile, sSect, sTemp);
			if (iInterval == -1)
				return FALSE;
		}

		PBUSDATA pBusData = new BUSDATA;
		strcpy(pBusData->m_BusString, sbuffer);
		pBusData->m_BusInterval = iInterval;
		strcpy(pBusData->m_ProtocolDllPath, sDllPath);

		if (!bPause)
		{
			// 获取通讯口其它通讯参数
			if (!AddPortOtherPara(pBusData, Profile, sbuffer, i + 1))
			{
				delete pBusData;
				return FALSE;
			}

			// 添加总线协议打印信息
			strcpy(pBusData->m_szPrintNetCard, NetCard);
			strcpy(pBusData->m_szPrintRemoteIp, RemoteIp);
			pBusData->m_dwPrintStartPortNum = StartPortNum;
		}

		bus.AddBusString(pBusData);

		// 打印信息
		PrintConfigMsg(i + 1, pBusData);
	}
	return TRUE;
}

void PrintConfigMsg(BYTE lineNo, PBUSDATA pBusData)
{ /*{{{*/
	switch (pBusData->m_BusType)
	{
	case COMRS232:
	case COMRS422:
	case COMRS485:
		printf("Bus Config%02d = %s ProtoType = %d  Interval = %d OK . \n",
			   lineNo, pBusData->m_BusString, pBusData->m_BusType, pBusData->m_BusInterval);
		break;
	case SOCKETTCP:

		printf("Bus Config%02d = %s ProtoType = %d  Interval = %d NetCardName = %s OK . \n",
			   lineNo, pBusData->m_BusString, pBusData->m_BusType,
			   pBusData->m_BusInterval, pBusData->m_NetCardName);
		break;
	case TCP_CLIENT:
		printf("Bus Config%02d = %s  ProtoType = %d  Interval = %d ServerIP = %s OK . \n",
			   lineNo, pBusData->m_BusString, pBusData->m_BusType,
			   pBusData->m_BusInterval, pBusData->m_szIP);
		break;
#ifdef LORA
	case LORA_WIRELESS:
		printf("Bus Config%02d = %s  ProtoType = %d  Interval = %d ServerIP = %s OK . \n",
			   lineNo, pBusData->m_BusString, pBusData->m_BusType,
			   pBusData->m_BusInterval, pBusData->m_szIP);
		break;
#endif //  LORA
	}
} /*}}}*/

BOOL AddPortOtherPara(PBUSDATA pBusData, CProfile &profile, char *busString, BYTE byNo)
{ /*{{{*/
	char sAttrib[100];
	UINT nPort = 0;
	BYTE byType = CBasePort::GetCommAttrib(busString, sAttrib, nPort);
	BOOL bflag = TRUE;

	switch (byType)
	{
	case COMRS232:
	case COMRS422:
	case COMRS485:
		break;
	case SOCKETTCP:
		// 读取子网掩码，网关，DNS
		bflag = AddNetPara(pBusData, profile, byNo);
		// 通过网卡名称获取IP地址

		// if( bflag )
		// strcpy( pBusData->m_szIP , sAttrib ) ;
		break;
	case TCP_CLIENT:
	case TCP_CLIENT_SHORT:
		strcpy(pBusData->m_szIP, sAttrib);
		break;
	case CAN_NET:

		break;
	default:
		return 0xFF;
	}

	pBusData->m_BusType = byType;
	return bflag;
} /*}}}*/

BOOL AddNetPara(PBUSDATA pBusData, CProfile &profile, BYTE byNo)
{ /*{{{*/
	if (pBusData == NULL || !profile.IsValid())
		return FALSE;

	/*
	   char sDNS[ ] = "DNS" ;
	   char sGateWay[ ] = "GateWay" ;
	   char sSubNetMask[ ] = "SubNetMask";
	   memset( sKey , 0 , sizeof( sKey ) ) ;
	   sprintf( sKey , "%s%02d" , sDNS , byNo ) ;

	//获取DNS
	if( !GetBusProfileString( profile , sSect , sKey , sTemp , sizebuff ) )
	return FALSE ;

	strcpy( pBusData->m_szLocalDNS , sTemp ) ;

	//获取网关
	sprintf( sKey , "%s%02d" , sGateWay , byNo ) ;
	memset( sTemp , 0 , sizeof( sTemp ) ) ;
	if( !GetBusProfileString( profile , sSect , sKey , sTemp , sizebuff ) )
	return FALSE ;

	strcpy( pBusData->m_szLocalGateWay , sTemp ) ;

	//获取子网掩码
	sprintf( sKey , "%s%02d" , sSubNetMask , byNo ) ;
	memset( sTemp , 0 , sizeof( sTemp ) ) ;
	if( !GetBusProfileString( profile , sSect , sKey , sTemp , sizebuff ) )
	return FALSE ;

	strcpy( pBusData->m_szLocalSubNetMask , sTemp ) ;
	*/
	char sNetCard[] = "NetCard";
	char sSect[] = "PORT";
	char sTemp[200] = {0};
	char sKey[200] = {0};
	int sizebuff = sizeof(sTemp);
	// 获取网卡名字
	sprintf(sKey, "%s%02d", sNetCard, byNo);
	memset(sTemp, 0, sizeof(sTemp));
	if (!GetBusProfileString(profile, sSect, sKey, sTemp, sizebuff))
		return FALSE;

	strcpy(pBusData->m_NetCardName, sTemp);

	return TRUE;
} /*}}}*/

BOOL GetBusProfileString(CProfile &Profile, char *sSect, char *sKey, char *sTemp, int &size)
{ /*{{{*/
	if (!Profile.IsValid())
		return FALSE;

	Profile.GetProfileString(sSect, sKey, (char *)"NULL", sTemp, size);
	if (strcmp("NULL", sTemp) == 0)
		return FALSE;

	return TRUE;
} /*}}}*/

int GetBusProtoInterval(CProfile &Profile, char *sSect, char *sKey)
{ /*{{{*/
	int wVal = 0;

	wVal = Profile.GetProfileInt(sSect, sKey, -1);
	if (wVal == -1)
	{
		printf("Get %s Error OK. \n", sKey);
		return wVal;
	}
	return wVal;
} /*}}}*/

BOOL InitBusLine()
{ /*{{{*/
	INITBUS bus;
	/*读取总线参数*/
	if (ReadPortPara(bus))
		printf("ReadPortPara OK.\n");
	else
	{
		printf("ReadPortPara Failed. \n ");
		return FALSE;
	}

	// create bus object
	if (!CreateBusLine(bus))
	{
		printf("CreateBusLine Failed. \n ");
		return FALSE;
	}

	// fault recorder
	setupFaultRecorder();

	// communation thread
	if (!InitComThread())
	{
		printf("InitComThread Failed. \n ");
		return FALSE;
	}

	// 初始化打印报文消息队列
	InitPrintMsgQueue();

	////Devcheck
	// CSocketDevCheck *pDevCheck = NULL;
	// CreateSocketDevCheck(&pDevCheck);

	return TRUE;
} /*}}}*/

void InitPrintMsgQueue()
{
	const DWORD channel = 20190830;
	g_printMsgQueue.CreateMsgQueue(channel);

	g_printMsgTcpPort.m_uThePort = 50000;
	strcpy(g_printMsgTcpPort.m_szAttrib, "127.0.0.1");
	char buf[100] = {0};
	g_printMsgTcpPort.OpenPort(buf);
}

BOOL setupFaultRecorder()
{
	if (!m_busManager.setupFaultRecorder())
		cout << "fault recorder setup failing" << endl;
	return TRUE;
}

BOOL SetLocalNetPara(CProfile &profile)
{ /*{{{*/
	char sNetCard[] = "NetCard";
	char sSect[] = "NetCard";
	char sDNS[] = "DNS";
	char sGateWay[] = "GateWay";
	char sSubNetMask[] = "SubNetMask";
	char sIP[] = "IP";
	char sRouteIp[] = "RouteIp";

	char sTemp[200] = {0};
	int sizebuff = sizeof(sTemp);

	char sSysDNS[] = "SYSTEM-DNS";
	char sSysDNS_Key[] = "DNS";
	char sTemp_Dns[20] = {0};
	int sizebuff_Dns = sizeof(sTemp_Dns);

	char sKey[200] = {0};

	for (BYTE byNo = 1; byNo < 5; byNo++)
	{
		memset(sKey, 0, sizeof(sKey));
		sprintf(sKey, "%s%02d", sDNS, byNo);
		NETWORKPARAM netParam;

		// 获取DNS
		// if( !GetBusProfileString( profile , sSect , sKey , sTemp , sizebuff ) )
		// return FALSE ;

		strcpy(netParam.sDNS, sTemp);

		// 获取网关
		sprintf(sKey, "%s%02d", sGateWay, byNo);
		memset(sTemp, 0, sizeof(sTemp));
		if (!GetBusProfileString(profile, sSect, sKey, sTemp, sizebuff))
			return FALSE;

		strcpy(netParam.sGateWay, sTemp);

		// 获取子网掩码
		sprintf(sKey, "%s%02d", sSubNetMask, byNo);
		memset(sTemp, 0, sizeof(sTemp));
		if (!GetBusProfileString(profile, sSect, sKey, sTemp, sizebuff))
			return FALSE;

		strcpy(netParam.sSubNetMask, sTemp);

		// 获取网卡名字
		sprintf(sKey, "%s%02d", sNetCard, byNo);
		memset(sTemp, 0, sizeof(sTemp));
		if (!GetBusProfileString(profile, sSect, sKey, sTemp, sizebuff))
			return FALSE;

		strcpy(netParam.pNetCardName, sTemp);

		// 获取网卡IP
		sprintf(sKey, "%s%02d", sIP, byNo);
		memset(sTemp, 0, sizeof(sTemp));
		if (!GetBusProfileString(profile, sSect, sKey, sTemp, sizebuff))
			return FALSE;

		strcpy(netParam.sIp, sTemp);

		// 获取网卡IP
		sprintf(sKey, "%s%02d", sRouteIp, byNo);
		memset(sTemp, 0, sizeof(sTemp));
		if (!GetBusProfileString(profile, sSect, sKey, sTemp, sizebuff))
		{
			printf("%s can'find\n", sKey);
			memset(netParam.sRouteIp, 0, sizeof(netParam.sRouteIp));
		}
		else
		{
			strcpy(netParam.sRouteIp, sTemp);
		}
		m_busManager.SetNetCardParam(&netParam);
	}

	if (!GetBusProfileString(profile, sSysDNS, sSysDNS_Key, sTemp_Dns, sizebuff_Dns))
	{
		return FALSE;
		/*end*/
	}
	else
	{
		strcpy(m_busManager.m_sysDns, sTemp_Dns);
	}

	m_busManager.EnableNetCardParam();
	return TRUE;
} /*}}}*/

BOOL InitComThread()
{ /*{{{*/
	int size = m_busManager.m_sbus.size();
	if (size == 0)
		return FALSE;

	for (int i = 0; i < size; i++) // 一条总线一个线程!
	{
		PBUSMANAGER pBus = m_busManager.m_sbus[i];
		THREADPARA ThreadPara;

		// 判断总线是否是PAUSE
		if (!pBus->m_Protocol || !pBus->m_pMethod || !pBus->m_Port)
			continue;

		// 打开通讯总线端口
		if (!OpenPort(pBus))
		{
			LogPromptText("Open Port = %d Failed ! \n ", pBus->m_Port->m_uThePort);
			LogPromptText("pBus->hThread = %d \n", pBus->hThread);
			continue;
		}
		else
		{
			LogPromptText("Open Port = %d Success ! \n ", pBus->m_Port->m_uThePort);
		}

		ThreadPara.hThread = pthread_create(&ThreadPara.ThreadID, NULL, ThreadProc, (void *)pBus);
		if (ThreadPara.ThreadID < 0)
		{
			OutPromptText((char *)" ****CreateThread Fail!****");
		}
		pBus->hThread = ThreadPara.hThread;
		pBus->ThreadID = ThreadPara.ThreadID;
	}
	// 扫描Client端是否掉线
	ScanServerOnLine();

	return TRUE;
} /*}}}*/

void ScanServerOnLine()
{ /*{{{*/
	int size = m_busManager.GetClientSize();
	if (size == 0)
		return;

	THREADPARA ThreadPara;
	ThreadPara.hThread = pthread_create(&ThreadPara.ThreadID, NULL, ThreadScanSever, (void *)m_busManager.GetSanServerVector());
	if (ThreadPara.ThreadID < 0)
	{
		OutPromptText((char *)" ****ScanServerOnLine____CreateThread Fail!****");
	}

	return;
} /*}}}*/

void *ThreadScanSever(void *pProtObj)
{ /*{{{*/
	vector<CBasePort *> *pVectorScan = (vector<CBasePort *> *)pProtObj;
	if (pVectorScan == NULL)
		return NULL;

	int size = pVectorScan->size();
	if (!size)
		return NULL;

    // 61850网关临时增加一个count 变量，因为一般只开一个client,当此client 一直连不上时，重启网卡操作，或以后连接多装置，将count 改为数组
    int conn_count = 0;
    int conn_max = 120;
    int eth1_exist = 0;

	while (g_bAppRun)
	{
		vector<CBasePort *>::iterator ibegin = pVectorScan->begin();
		vector<CBasePort *>::iterator iend = pVectorScan->end();

        printf("11111\n");
        char ip[64] = "eth1";
        if (!m_busManager.m_Debug.GetLocalIp(ip) ){
            printf("11112\n");
            eth1_exist = 0;
        }
        else {
            conn_count++;
            eth1_exist = 1;
            printf("11113 local ip=%s\n", ip);
        }

		for (; ibegin != iend; ibegin++)
		{
            bool b = ( *ibegin )->Ping( ( *ibegin )->m_szRemoteAddr ) ;
            if (!b) {
                printf("ping %s error\n", ( *ibegin )->m_szRemoteAddr);
                conn_count++;
                continue;
            }

            if (((*ibegin)->IsPortValid()) ||
                ((*ibegin)->m_uThePort == 10086)) {
                conn_count = 0;
                continue;
            }
                // if( ( *ibegin )->IsPortValid() )
			printf("ScanServerOnLine____%s:%d is not online, try to open it again!\n", (*ibegin)->m_szRemoteAddr, (*ibegin)->m_uThePort);
			(*ibegin)->ClosePort();
            sleep(1);
            b = (*ibegin)->OpenPort();
            if (!b) {
                printf("ScanServerOnLine____OpenPort %s:%d error\n", (*ibegin)->m_szRemoteAddr, (*ibegin)->m_uThePort);
            }
            sleep(1);
            conn_count++;
		}

        // if (conn_count > 3) {
            printf("ScanServerOnLine____%d times failed! max=%d\n", conn_count, conn_max);
        // }
        // 61850网关独有
        if (conn_count >= conn_max || eth1_exist == 0) {
            conn_count = 0;
            printf("ScanServerOnLine____%d times failed, try to restart network card!\n", conn_count);
            // system("/root/gpout_test 1 1");

            int gp_fd = -1;
            char data[2] = {1,1};

            gp_fd = open("/dev/esd_gpout", O_WRONLY);
            if (gp_fd == -1) {
                printf("fail to open /dev/esd_gpout !\n");
                Asleep(2000);
                continue;
            }

            if(write(gp_fd, data, sizeof(data)) > 0)//传入的数据长度必须为2
                printf("main.c reset lan3 data %d level %d\n", data[0], data[1]);

            close(gp_fd);

            Asleep(2000);

            char sBusLine[] = BUS_PATH;
            CProfile Profile(sBusLine);

            /*设置本地相应网络参数*/
            if (!SetLocalNetPara(Profile)) {
                continue;
            }
        }

		Asleep(1000);
	}
	printf("Scan Client thread has exited!\n");
	return NULL;
} /*}}}*/

BOOL OpenPort(const PBUSMANAGER pBus)
{ /*{{{*/
	if (pBus == NULL)
		return FALSE;

	CBasePort *pBasePort = pBus->m_Port;
	if (pBasePort == NULL)
		return FALSE;

	char szBuf[96];
	if (!pBasePort->OpenPort(szBuf))
	{
		LogPromptText(szBuf);
        printf("%s\n", szBuf);
		return FALSE;
	}

	return TRUE;
} /*}}}*/

const char *GetProtocolName(char *sProtocol)
{ /*{{{*/
	if (sProtocol == NULL)
		return NULL;

	string strName(sProtocol);
	int pos1 = strName.find_last_of("lib");
	int pos2 = strName.find_last_of(".so");
	return strName.substr(pos1 + 1, pos2 - pos1 - 3).data();
} /*}}}*/

BOOL CreateBusLine(INITBUS &bus)
{ /*{{{*/
	int size = bus.m_busData.size();

	if (size == 0)
	{
		printf("CreateBusLine In ReadProtPara Failed . \n ");
		return FALSE;
	}
	WORD wGatherDevCount = 0;
	// 创建协议对象,i表示总线号!
	for (int i = 0; i < size; i++)
	{
		char sComType[100];
		UINT nPort = 0;
		memset(sComType, 0, sizeof(sComType));
		BYTE byComType = 0;
		CBasePort *pPort = NULL;
		CProtocol *pProtocol = NULL;
		WORD wInterval = 0;
		PBUSDATA pBusData = bus.m_busData[i];
		BYTE gather_bus_num = 0;

		if ((strcmp(pBusData->m_BusString, PASUE) == 0) && pBusData->m_BusType == COM_PAUSE)
		{
			m_busManager.AddPauseBus();
			continue;
		}

		CMethod *pMethod = new CPublicMethod;
		pMethod->m_pRdbObj = g_pRTDBObj;
		pMethod->m_pBusManager = &m_busManager;

		byComType = CBasePort::GetCommAttrib(pBusData->m_BusString, sComType, nPort);
		pPort = InitCom(byComType);
		pProtocol = InitProtocol(pBusData->m_ProtocolDllPath, pMethod);
		if (NULL == pProtocol) // 2015年04月14日 11时15分42秒 mengqp 增加pProtocol有效性判断 错误直接返回
		{
			return FALSE;
		}
		wInterval = pBusData->m_BusInterval;

		if (nPort == 0 && byComType != CAN_NET && byComType != LORA_WIRELESS)
		{
			printf("Open %s Error  OK . \n", pBusData->m_BusString);
			continue;
		}
		if (pProtocol == NULL)
		{
			printf("Create %s Failed OK . \n ", pBusData->m_ProtocolDllPath);
			continue;
		}

		// 设置通讯口属性
		if (byComType == SOCKETTCP)
		{
			// 获取对应网卡IP地址
			NETWORKPARAM netParam;
			strcpy(netParam.pNetCardName, pBusData->m_NetCardName);
			if (!m_busManager.GetNetCardParam(netParam.pNetCardName, &netParam))
			{
				printf("GetNetParam Error！NetCardName = %s\n", pBusData->m_NetCardName);
				return FALSE;
			}

			strcpy(sComType, netParam.sIp);
		}

		pPort->m_uThePort = nPort;
		sprintf(pPort->m_szAttrib, "%s", sComType);

		// 设置串口工作模式
		SetSerialPortMode(byComType, nPort);

		// 保存端口信息
		pMethod->m_pPort = pPort;

		// 初始化协议中模块数据
		if (pProtocol->Init(i))
			m_busManager.AddBus(pProtocol, pPort, wInterval, i + 1, pMethod, pBusData->m_szPrintNetCard, pBusData->m_szPrintRemoteIp, pBusData->m_dwPrintStartPortNum);

		wGatherDevCount += pMethod->GetSingleGatherDevCount(i);

		char szComBuf[200] = {0};
		sprintf(szComBuf, "%s:%d-%s", GetBusType(byComType), nPort, GetProtocolName(pBusData->m_ProtocolDllPath));
		// printf("---------byComType=%d----%s----------------%s-----%d", byComType, GetBusType(byComType), szComBuf, i);
		strcpy(g_pRTDBObj->m_pRTDBSpace->RTDBase.StnComStatus.byBusTypeAndProtocolName[i], szComBuf);
	}

	// 二次初始化装置数据
	InitDevState(wGatherDevCount);
	return TRUE;
} /*}}}*/

char *GetBusType(BYTE byComType)
{
	if (byComType == COMRS232)
		return "COM232";

	else if (byComType == COMRS485)
		return "COM485";

	else if (byComType == COMRS422)
		return "COM422";

	else if (byComType == SOCKETTCP)
		return "TCP_SERVER";

	else if (byComType == TCP_CLIENT)
		return "TCP_CLIENT";

	else if (byComType == TCP_CLIENT_SHORT)
		return "TCP_CLIENT_SHORT";

	else if (byComType == CAN_NET)
		return "CAN_NET";

	else if (byComType == LORA_WIRELESS)
		return "LORA";
	else
		return 0;
}

BOOL InitDevState(WORD wDevCount)
{ /*{{{*/
	// 采集总线的总装置个数设置到每个PublicMethod中
	int size = m_busManager.m_sbus.size();
	for (int i = 0; i < size; i++)
	{
		PBUSMANAGER pBusManager = m_busManager.m_sbus[i];
		if (pBusManager->m_pMethod)
			pBusManager->m_pMethod->SetGatherDevCount(wDevCount);

		CProtocol *pExternalProto = pBusManager->m_Protocol;
		if (pExternalProto == NULL)
			continue;

		// 二次初始化每个装置数据
		int devSize = pExternalProto->m_module.size();
		for (int m = 0; m < devSize; m++)
		{
			CProtocol *pProtocol = pExternalProto->m_module[m];
			pProtocol->InitDevState();
		}
	}

	return TRUE;
}; /*}}}*/

// 设置串口工作模式
BOOL SetSerialPortMode(BYTE byComType, DWORD nPort)
{ /*{{{*/
	switch (byComType)
	{
	case COMRS232:
	{
		if (nPort == 1 || nPort == 2)
			SetSerialConfigMode(nPort);
	}
	break;
	case COMRS422:
	{
		if (nPort == 8 || nPort == 4)
			SetSerialConfigMode(nPort);
	}
	break;
	default:
		return FALSE;
	}

	return TRUE;
} /*}}}*/

CProtocol *InitProtocol(char *pDllPath, CMethod *pMethod)
{ /*{{{*/
	CProtocol *pProtocol = NULL;
	pProtocol = m_busManager.m_GetProtocol->GetProtoObj(pDllPath, pMethod);
	return pProtocol;
} /*}}}*/

CBasePort *InitCom(BYTE byComType)
{ /*{{{*/
	CBasePort *pPort = NULL;

	switch (byComType)
	{
	case COMRS232:
	case COMRS422:
	case COMRS485:
		pPort = new CSerialPort;
		break;
	case SOCKETTCP:
		pPort = new CTcpPortServer;
		break;
	case TCP_CLIENT:
	{
		pPort = new CTcpClient;
		m_busManager.AddClientPort(pPort);
	}
	break;
	case TCP_CLIENT_SHORT:
	{
		pPort = new CTcpClientShort;
	}
	break;
#ifdef ARM
	case CAN_NET:
	{
		printf("new CCannet \n");
		pPort = new CCanNet;
	}
	break;

#endif

#ifdef LORA
	case LORA_WIRELESS:
		pPort = new CLoraPort;
		break;
#endif

	default:
		return NULL;
	}
	return pPort;
} /*}}}*/

void Asleep(DWORD dwMilliSecd)
{ /*{{{*/
	struct timeval tv;
	tv.tv_sec = 0;
	tv.tv_usec = dwMilliSecd * 1000;
	select(0, NULL, NULL, NULL, &tv);
} /*}}}*/

#define RECV_INTE 1 /*  */

BOOL ProcessRealData(PBUSMANAGER pbus, int &index, int size, PBUSMSG pBusMsg)
{ /*{{{*/
	const BYTE byMaxRxErrFrameNum = 3;
	char chDebugBuf[512];
	if (pbus == NULL)
		return FALSE;
	if (!pbus->m_Protocol || !pbus->m_pMethod || !pbus->m_Port)
	{
		Asleep(100);
		return FALSE;
	}

	if (pbus->m_Protocol->m_ProtoType == PROTOCO_TRANSPROT)
	{
		if (pbus->m_Protocol->m_TransDelay > 0)
		{
			BYTE DDBBusNo;
			WORD DDBAddr;
			CPublicMethod::GetDDBDevBusAndAddr(DDBBusNo, DDBAddr);
			if (DDBBusNo != pbus->byBusNo)
				Asleep(1000 * pbus->m_Protocol->m_TransDelay);
			pbus->m_Protocol->m_TransDelay = 0;
		}
	}
	CProtocol *pProtocol = NULL;
	CBasePort *pPort = pbus->m_Port;
	if (index >= size)
		index = 0;
	pProtocol = pbus->m_Protocol->m_module[index++];
	if (pProtocol == NULL)
	{
		usleep(100 * 1000);
		printf("pProtocol == NULL Failed \n");
		cout << __FUNCTION__ << "  " << __LINE__ << endl;
		return FALSE;
	}
	setComState(pProtocol, pPort);

	// 判断双机冗余机制
	BOOL bDDB = FALSE;
	if (CPublicMethod::IsHaveDDB() &&
		(CPublicMethod::GetDDBSyncState() == STATUS_SLAVE) &&
		pProtocol->m_ProtoType == PROTOCO_GATHER)
		bDDB = TRUE;

	BYTE buf[MAX_BUFFER];
	memset(buf, 0, MAX_BUFFER);
	int len = 0;
	if (!bDDB)
	{

		if (pProtocol->GetProtocolBuf(buf, len, pBusMsg))
			if (len > 0)
			{
				int wLen = pPort->WritePort(buf, len);
				if (wLen > 0 && wLen < 1024)
					OutBusDebug(pbus->byBusNo, buf, wLen, 0);
				pbus->m_Tx++;
				if (pbus->m_Tx > 0xFFFF)
					pbus->m_Tx = 0;
			}
	}
	// 单位毫秒
	if (bDDB)
		Asleep(100);
	else
		Asleep(pbus->wInterval);

	len = sizeof(buf);
	int ReadLen = pPort->AsyReadData(buf, len);
	if (ReadLen <= 0)
	{

		setDevComState(pProtocol, COM_DEV_ABNORMAL);
		if (COM_DEV_NORMAL == (pProtocol->GetDevCommState())) // 主要是针对61850的判断
			setDevComState(pProtocol, COM_DEV_NORMAL);

		char arrayTimeStr[100] = {0};
		getCurrentTimeStr(arrayTimeStr);

		sprintf(chDebugBuf, "%s bus_line:%d Addr:%d State:abnormal.port error!!",
				arrayTimeStr,
				pProtocol->m_byLineNo + 1,
				pProtocol->m_wDevAddr);
		OutBusDebug(pbus->byBusNo, (BYTE *)chDebugBuf, strlen(chDebugBuf), 2);
		return FALSE;
	}
	if (!bDDB)
	{

		OutBusDebug(pbus->byBusNo, buf, ReadLen, 1);
		if (pProtocol->ProcessProtocolBuf(buf, ReadLen))
		{
			setDevComState(pProtocol, COM_DEV_NORMAL);
			pbus->m_Rx++;
			if (pbus->m_Rx > 65535)
				pbus->m_Rx = 0;

			char arrayTimeStr[100] = {0};
			getCurrentTimeStr(arrayTimeStr);
			pbus->m_RxError = 0;

			sprintf(chDebugBuf, "%s BusLine:%d Addr:%d State:normal\n correct:%u err:%u",
					arrayTimeStr,
					pProtocol->m_byLineNo + 1,
					pProtocol->m_wDevAddr, pbus->m_Rx, pbus->m_RxError);

			OutBusDebug(pbus->byBusNo, (BYTE *)chDebugBuf, strlen(chDebugBuf), 2);
		}
		else
		{

			pbus->m_RxError++;
			if (pbus->m_RxError >= 65535)
				pbus->m_RxError = 0;

			char szState[20] = {0};
			if (pbus->m_RxError > 5)
			{
				setDevComState(pProtocol, COM_DEV_ABNORMAL);
				strcpy(szState, "abnormal");
			}
			else
			{
				setDevComState(pProtocol, COM_DEV_NORMAL);
				strcpy(szState, "normal");
			}

			char arrayTimeStr[100] = {0};
			getCurrentTimeStr(arrayTimeStr);

			sprintf(chDebugBuf, "%s bus_line:%d Addr:%d State:%s\n  correct:%u err:%u",
					arrayTimeStr,
					pProtocol->m_byLineNo + 1,
					pProtocol->m_wDevAddr, szState, pbus->m_Rx, pbus->m_RxError);
			OutBusDebug(pbus->byBusNo, (BYTE *)chDebugBuf, strlen(chDebugBuf), 2);
		}
	}

	return TRUE;
} /*}}}*/

void setComState(CProtocol *pProto, CBasePort *pPort)
{
	if (!pPort || !pProto)
		return;

	if (pProto->m_ProtoType != PROTOCO_GATHER)
		return;

	BYTE byPortNo = pProto->m_byLineNo;
	if (pPort->IsPortValid())
	{
		pProto->m_pMethod->m_pRdbObj->m_pRTDBSpace->RTDBase.StnComStatus.byBusComStatus[byPortNo] = COM_NORMAL;
	}
	else
	{
		pProto->m_pMethod->m_pRdbObj->m_pRTDBSpace->RTDBase.StnComStatus.byBusComStatus[byPortNo] = COM_ABNORMAL;
	}
}

void setDevComState(CProtocol *pProto, BOOL state)
{
	if (!pProto)
		return;

	if (pProto->m_ProtoType != PROTOCO_GATHER)
		return;

	WORD wSerialNo = pProto->m_SerialNo;
	pProto->m_pMethod->m_pRdbObj->m_pRTDBSpace->RTDBase.StnComStatus.byDevComStatus[wSerialNo] = state;
}

void SendPacketloss(PBUSMANAGER pbus)
{ /*{{{*/
	if (pbus == NULL)
		return;

	// 固定将编号为第13个装置作为转发每个通道丢包数的虚拟装置--仅仅作为测试使用
	if (pbus->m_Protocol->m_ProtoType == PROTOCO_TRANSPROT)
		return;

	DWORD dwLoss = 0;
	dwLoss = pbus->m_Tx - pbus->m_Rx;

	WORD wPnt = pbus->byBusNo + 52;
	pbus->m_pMethod->SetYcData(2, wPnt, dwLoss);
} /*}}}*/

PBUSMANAGER GetBusManagerPointer(BYTE byNo)
{ /*{{{*/
	int size = m_busManager.m_sbus.size();
	if (byNo >= size)
		return NULL;

	PBUSMANAGER pBus = m_busManager.m_sbus[byNo];
	if (!pBus->m_Protocol || !pBus->m_pMethod || !pBus->m_Port)
		return NULL;

	return pBus;
} /*}}}*/

BOOL ProcessSpecialMsg(PBUSMSG BusMsg, PBUSMANAGER Bus, int size)
{ /*{{{*/
	PBUSMSG pBusMsg = BusMsg;
	PBUSMANAGER pBus = Bus;

	if (pBusMsg == NULL || pBus == NULL)
	{
		printf("ProcessSpecialMsg failure\n");
		return FALSE;
	}

	if (!pBus->m_Protocol || !pBus->m_pMethod || !pBus->m_Port)
	{
		printf("ProcessSpecialMsg failure\n");
		return FALSE;
	}

	BOOL bFlag = FALSE;
	CProtocol *pProtocol = pBus->m_Protocol;
	BYTE byMsgType = pBusMsg->byMsgType;

	switch (byMsgType)
	{
	case BROADCASET_PROTO:
	{ /*{{{*/
		if (pProtocol)
		{
			BYTE buf[1024];
			memset(buf, 0, sizeof(buf));
			int len = 0;
			bFlag = pProtocol->BroadCast(buf, len);
			if (len > 0 && bFlag)
				pBus->m_Port->WritePort(buf, len);
		}
	} /*}}}*/
	break;
	case YK_PROTO:
	{ /*{{{*/
		WORD wDevAddr = pBusMsg->DstInfo.wDevNo;
		int iModuleNo = pProtocol->GetModuleNo(wDevAddr);

		if (iModuleNo == -1)
		{
			printf("can't find the Addr=%d\n", wDevAddr);
			break;
		}
		bFlag = ProcessRealData(pBus, iModuleNo, size, pBusMsg);
	} /*}}}*/
	break;
	case DZ_PROTO:
	{ /*{{{*/
		WORD wDevAddr = pBusMsg->DstInfo.wDevNo;
		int iModuleNo = pProtocol->GetModuleNo(wDevAddr);

		if (iModuleNo == -1)
		{
			printf("can't find the Addr=%d\n", wDevAddr);
			break;
		}
		bFlag = ProcessRealData(pBus, iModuleNo, size, pBusMsg);
	} /*}}}*/
	break;
	case UNVARNISH_PROTO:
	{
		WORD wDevAddr = pBusMsg->DstInfo.wDevNo;
		int iModuleNo = pProtocol->GetModuleNo(wDevAddr);

		if (iModuleNo == -1)
		{
			printf("can't find the Addr=%d\n", wDevAddr);
			break;
		}
		bFlag = ProcessRealData(pBus, iModuleNo, size, pBusMsg);
	}
	break;
	case THREAD_EXIT:
		pBus->m_bThreadRun = FALSE;
		printf("thread %d recv exit msg\n", pBus->m_Protocol->m_byLineNo);
		break;
	}
	return bFlag;
} /*}}}*/

/*
 * ===  FUNCTION  ======================================================================
 *         Name:  SendBusThreadExitMsg
 *  Description:  向各个总线发送线程退出消息
 * =====================================================================================
 */
static void SendBusThreadExitMsg()
{ /*{{{*/
	int size = m_busManager.m_sbus.size();
	PBUSMANAGER pBusManager = NULL;

	for (int i = 0; i < size; i++)
	{
		pBusManager = GetBusManagerPointer(i);
		if (pBusManager == NULL)
			continue;

		// printf ( "send thread exit msg size=%d bus%d pBusManager->hThread =%d sendmsg \n", size, i, pBusManager->hThread );
		if (pBusManager->hThread != -1)
		{
			PBUSMSG busMsg = new BUSMSG;
			busMsg->byMsgType = THREAD_EXIT;

			LMSG msg;
			msg.pVoid = busMsg;
			pBusManager->SendMsg(&msg);

			// printf ( "send thread exit msg size=%d bus%d sendmsg \n", size, i );
		}

		pBusManager = NULL;
	}

	return;
} /* -----  end of static function SendBusThreadExitMsg  ----- */ /*}}}*/

/*
 * ===  FUNCTION  ======================================================================
 *         Name:  WaitThreadexit()
 *  Description:  等待线程退出  最大为5s
 * =====================================================================================
 */
static void WaitThreadexit()
{ /*{{{*/
	int nTicks = 0;
	BOOL bThreadExit = FALSE;
	int i;
	PBUSMANAGER pBusManager = NULL;

	int size = m_busManager.m_sbus.size();
	while (nTicks < THREAD_EXIT_TIME)
	{
		for (i = 0; i < size; i++)
		{
			pBusManager = GetBusManagerPointer(i);
			if (pBusManager == NULL)
				return;

			if (pBusManager->m_bThreadRun == TRUE)
			{
				bThreadExit = FALSE;
				break;
			}

			pBusManager = NULL;
		}

		if (i >= size)
		{
			bThreadExit = TRUE;
			break;
		}

		Asleep(100);
		nTicks++;
	}

	if (!bThreadExit)
	{
		for (i = 0; i < size; i++)
		{
			pBusManager = GetBusManagerPointer(i);
			if (pBusManager == NULL)
				return;
			if (pBusManager->m_bThreadRun == TRUE)
			{
				// printf ( "Thread %d not exit we will exit the thread forcely\n",i);
				pthread_cancel(pBusManager->ThreadID);
				pthread_join(pBusManager->ThreadID, NULL);
			}
			pBusManager = NULL;
		}
	}
	else
	{
		printf("All Thread exit\n");
	}
	return;
} /* -----  end of static function WaitThreadexit()  ----- */ /*}}}*/

void *ThreadProc(void *pProtObj)
{ /*{{{*/
	PBUSMANAGER pbus = (PBUSMANAGER)pProtObj;
	int size = pbus->m_Protocol->m_module.size();
	int index = 0;
	while (pbus->m_bThreadRun)
	{
		LMSG msgRecv;
		if (pbus->RecvMsg(&msgRecv))
		{
			PBUSMSG busMsg = (PBUSMSG)msgRecv.pVoid;
			if (busMsg == NULL)
				continue;

			ProcessSpecialMsg(busMsg, pbus, size);

			if (busMsg != NULL)
			{
				delete busMsg;
				busMsg = NULL;
			}
		}
		else // 处理实时消息
		{
			ProcessRealData(pbus, index, size);
		}

	}

	printf("Exit Thread Port = %d \n", pbus->m_Protocol->m_byLineNo);

	pthread_exit(0);
	return NULL;
} /*}}}*/

static void OnTimeProc()
{ /*{{{*/
	int nCount = m_busManager.m_sbus.size();
	int nMoudleSize = 0;
	PBUSMANAGER pBus;
	CProtocol *pProtObj = NULL;
	CProtocol *pMoudle;

	// 调用时间处理过程
	for (int i = 0; i < nCount; i++)
	{
		pBus = m_busManager.m_sbus[i];
		if (!pBus->m_Protocol || !pBus->m_pMethod || !pBus->m_Port)
			continue;

		pProtObj = pBus->m_Protocol;
		nMoudleSize = pProtObj->m_module.size();
		/* 时间处理函数	 */
		for (int k = 0; k < nMoudleSize; k++)
		{
			pMoudle = pProtObj->m_module[k];
			pMoudle->TimerProc();
		}
	}

	/*
		//check register file every 10minites
		static time_t begin = time( NULL ) ;
		time_t end = time(NULL ) ;
		if ((end - begin) > 10 * 60)
		{
			if (!checkRegisterFile())
			{
				time_t tInterval = time(NULL);
				if( tInterval %20 == 0 )
					printf("register file error\n");
				m_log.writeLog("check regfile failure! apps exits!");
				system("killall -9 pman gather rtdbserver boa webudp");
				exit(1);
			}
			time(&begin);
		}*/

} /*}}}*/

int getUniqueCode(char *pUniqueCode)
{

#define IO_SN_READ 0x1976
	int fd, res;
	char rsp[32];
	char mac[32];
	char random[32];
	char serno[32];
	fd = open("/dev/atsha0", O_RDWR);
	if (fd < 0)
	{
		err(1, "/dev/atsha0");
		return -1;
	}
	res = ioctl(fd, IO_SN_READ, serno);
	if (res)
	{
		printf("error read serno\n");
		close(fd);
		return -1;
	}
	else
	{
		for (int i = 0; i < 9; i++)
		{
			sprintf(&pUniqueCode[i * 2], "%02x", serno[i]);
		}
	}

	close(fd);
}

// 校验通过为True 校验没通道为False
bool checkRegisterFile()
{
	char filename[] = "/usr/reg.reg";
	if (access(filename, F_OK))
	{
		printf("register file does not exist\n");
		return false;
	}

	FILE *file = fopen(filename, "r");
	char szContent[100] = {0};
	int res = 0;
	res = fread(szContent, 32, 1, file);
	if (!res)
	{
		printf("register file reads error \n");
		fclose(file);
		return false;
	}
	fclose(file);

	// 获取设备唯一码
	char szUniqueCode[100] = {0};
	if (g_szUniqueCode.empty())
	{
		getUniqueCode(szUniqueCode);
		g_szUniqueCode = szUniqueCode;
	}

	memcpy(szUniqueCode, g_szUniqueCode.c_str(), g_szUniqueCode.length());
	m_log.writeLog(szUniqueCode);

	// 准备注册码
	if (!modifyCode(szUniqueCode))
		return false;

	// 生成MD5码
	char md5Buf[100] = {0};
	MD5(md5Buf, szUniqueCode);

	if (strcmp(szContent, md5Buf) != 0)
		return false;

	return true;
}

bool modifyCode(char *szUniqueCode)
{
	if (strlen(szUniqueCode) != 18)
		return false;

	char des[30] = {0};
	char *pToken = {"?-+-~qw1"};
	int m = 0, index = 0, n = 0;
	;
	for (int i = 0; i < 18;)
	{
		if (m == 0 || m == 1)
		{
			des[index++] = szUniqueCode[i];
			m++;
			i++;
		}
		else
		{
			des[index++] = pToken[n++];
			m = 0;
		}
	}

	memcpy(szUniqueCode, des, sizeof(des));
	return true;
}

void processPrintMsgQueue()
{
	LMSG msgRecv;
	if (!g_printMsgQueue.RecvMsg(&msgRecv))
		return;

	printf("%d %s/n", __LINE__, __FILE__);

	DWORD dwMsg = (DWORD)msgRecv.pVoid;
	if (dwMsg == 0)
		return;

	BYTE updMsg = dwMsg & 0xFF;
	BYTE updOpt = (dwMsg >> 8) & 0xFF;
	BYTE lowBusno = (dwMsg >> 16) & 0xFF;
	BYTE highBusno = (dwMsg >> 24) & 0xFF;
	WORD wBusNo = MAKEWORD(lowBusno, highBusno);

	printf("wBusNo=%d updOpt=%d udpMsg = %d  %d %s\n", wBusNo, updOpt, updMsg, __LINE__, __FUNCTION__);
	const BYTE PRINT_UDP_MSG = 1;
	const BYTE OPEN = 1;
	const BYTE CLOSE = 0;
	if (updMsg == PRINT_UDP_MSG)
	{
		if (updOpt == CLOSE)
			g_BusNo = 0xFFFF;
		else if (updOpt == OPEN)
			g_BusNo = wBusNo;
	}
}

int main(int argc, char **argv)
{ /*{{{*/

	if (already_running("/mynand/config/Baylayer.lock"))
	{
		printf("the Program is running \n");
		return -1;
	}
	else
		printf("the Program Begin to Run\n");

	if (argc >= 2)
		OutSerialSum((int)atoi(argv[1]));
	SignalHook(SIGKILL, SignHandler);
	SignalHook(SIGTERM, SignHandler);

	g_bAppRun = TRUE;
	SignalHook(SIGINT, SignHandler);  /*CTRL-C*/
	SignalHook(SIGQUIT, SignHandler); /*CTRL-\*/
	SignalHook(SIGSTOP, SignHandler); /*CTRL-Z*/

	m_log.setLogKey("gather");
	printf("----------------Open RTDBase----------------\n");
	if (Open_SHM_DBase() < 0)
	{
		exit(-1);
	}

	ShowRTDBInfo();

	/*初始化总线*/
	if (InitBusLine())
		printf("InitBusLine OK.\n");
	else
	{
		printf("InitBusLine Failed. \n ");
		exit(0);
	}

	m_log.writeLog("restart success");
	/**/
	// 不需要等待时间,通过发送消息让线程退出
	while (g_bAppRun)
	{
		processPrintMsgQueue();
		Asleep(200);
		OnTimeProc();
	}

	// 发送消息等待线程退出
	SendBusThreadExitMsg();
	WaitThreadexit();
	printf("Program End! \n\n\n");

	return 0;
} /*}}}*/
/*****************************************************************************/
