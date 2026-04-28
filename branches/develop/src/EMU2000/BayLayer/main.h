#ifndef MAIN_H_INCLUDED
#define MAIN_H_INCLUDED

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdarg.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/param.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <errno.h>
#include <signal.h>
#include "../share/UdpPort.h"

// #include "./modbus/modbus.h"
#include "../share/rdbFun.h"
#include "../share/CProtocol.h"
#include "../share/profile.h"
#include "../share/TcpListen.h"
#include "../share/CTcpPortServer.h"
#include "../share/SerialPort.h"
#include "../share/TcpClient.h"
#include "../share/TcpClientShort.h"
#include	"../share/Rtu.h"
#include	"../librtdb/rdbObj.h"
#include "GetProtocol.h"
#include "../share/cmsg.h"
#include "CBusDebug.h"
// #include "CPublicMethod.h"
#include <vector>
#include "globleDef.h"
#include "../share/global.h"

using namespace std ;

class CBusManger ;
class CPublicMethod;

//最大通讯行数据
#define MAX_LINE	200

//工程路径
#define BUS_PATH	"/mynand/config/BusLine.ini"

//总线结构
typedef struct tagBusData
{
	tagBusData( )
	{
		memset( m_BusString , 0 , sizeof( m_BusString ) ) ;
		m_BusType = 0 ;
		m_BusInterval = 0 ;
		//memset( m_szLocalGateWay , 0 , sizeof( m_szLocalGateWay ) );
		//memset( m_szLocalSubNetMask , 0 , sizeof( m_szLocalSubNetMask ) );
		//memset( m_szLocalDNS , 0 , sizeof( m_szLocalDNS ) ) ;

		memset( m_szPrintNetCard, 0, sizeof( m_szPrintNetCard ) );
		memset( m_szPrintRemoteIp, 0, sizeof( m_szPrintRemoteIp ) );
	}
	char m_BusString[ 200 ] ;
	int  m_BusType ;
	WORD  m_BusInterval ;
	//char    m_szLocalGateWay[24];
	//char    m_szLocalSubNetMask[24];
	//char    m_szLocalDNS[24];
	char   m_szIP[24];
	char m_NetCardName[ 30 ];
	char m_ProtocolDllPath[ 200 ] ;

	//端口打印报文相关  须在BusLine.ini 中配置
	char m_szPrintNetCard[5];    //网卡（eth0 eth1 eth2 eth3）
	char m_szPrintRemoteIp[16];  //打印的远程IP
	DWORD m_dwPrintStartPortNum;
}BUSDATA, *PBUSDATA;

//定义结构
typedef struct tagBusInfo
{
	public:
		tagBusInfo( )
		{
		}
		~tagBusInfo()
		{
			RemoveAll() ;
			//printf( "Bus Line Destruct OK . \n" ) ;
		}

		BOOL AddBusString( PBUSDATA pBusData )
		{
			if( pBusData == NULL )
				return FALSE ;

			m_busData.push_back( pBusData ) ;

			return TRUE ;
		}

		BOOL RemoveAll( )
		{
			int nCount = m_busData.size();
			if( nCount > 0 )
			{
				while(nCount--)
					delete m_busData[nCount];

				m_busData.clear();
			}
			return TRUE ;
		}

	public:
		vector<PBUSDATA> m_busData ;
}INITBUS , *PINITBUS ;

typedef struct tagBusManager
{
	CProtocol *  m_Protocol ;
	CBasePort * m_Port ;
	CBusDebug m_Debug;
	CMethod * m_pMethod ;
	WORD wInterval ; //总线间隔
	CMsg m_msg ;
	int hThread;
	pthread_t ThreadID ;
	BOOL m_bThreadRun;
	BYTE byBusNo ;
	DWORD m_Tx , m_Rx , m_RxError;
	tagBusManager( )
	{
		m_bThreadRun = TRUE;
		m_Protocol = NULL ;
		m_Port = NULL ;
		hThread = -1 ;
		ThreadID = 0 ;
		byBusNo = 0xFF;
		m_pMethod = NULL ;
		m_Tx = 0 ;
		m_Rx = 0 ;
		m_RxError = 0;
	}
	~tagBusManager( )
	{
		if( hThread >=0 )
		{
			pthread_join( ThreadID , 0 ) ;
			printf( "Release Thread Resource! \n" );
		}
	}

	BOOL SendMsg( void * pVoid  )
	{
		return m_msg.SendMsg( pVoid  );
	}

	BOOL RecvMsg( void * pVoid  )
	{
		return m_msg.RecvMsg( pVoid ) ;
	}

}BUSMANAGER, *PBUSMANAGER ;
typedef vector< PBUSMANAGER > BUSARRAY ;

typedef struct tagNetWorkParam
{
	char pNetCardName[ 50 ] ;
	char sDNS[ 50 ] ;
	char sGateWay[ 50 ]  ;
	char sSubNetMask[ 50 ] ;
	char sIp[ 50 ] ;
	char sRouteIp[ 50 ] ;

	tagNetWorkParam( )
	{
		memset( pNetCardName , 0 , sizeof( pNetCardName ) ) ;
		memset( sDNS , 0 , sizeof( pNetCardName ) ) ;
		memset( sGateWay , 0 , sizeof( pNetCardName ) ) ;
		memset( sSubNetMask , 0 , sizeof( pNetCardName ) ) ;
		memset( sIp , 0 , sizeof( pNetCardName ) ) ;
		memset( sRouteIp , 0 , sizeof( sRouteIp ) ) ;
	}

	BOOL operator <<( tagNetWorkParam * pParam )
	{
		if( pParam == NULL )
			return FALSE ;

		strcpy( pNetCardName , pParam->pNetCardName ) ;
		strcpy( sDNS , pParam->sDNS ) ;
		strcpy( sGateWay , pParam->sGateWay ) ;
		strcpy( sSubNetMask , pParam->sSubNetMask ) ;
		strcpy( sIp , pParam->sIp ) ;
		strcpy( sRouteIp , pParam->sRouteIp ) ;

		return TRUE ;
	}

}NETWORKPARAM , *PNETWORKPARAM ;
typedef vector< NETWORKPARAM > NETWORKPARAM_ARRAY ;

//##ModelId=53CDE1B003D9
typedef struct tagThreadPara
{
	int hThread;
	pthread_t ThreadID ;
}THREADPARA , *PTHREADPARA ;
typedef vector< THREADPARA > THREADARRAY ;

//定义声明函数
BOOL InitBusLine( ) ;
void InitPrintMsgQueue();
BOOL setupFaultRecorder();
int GetBusProtoInterval(CProfile &Profile, char * sSect, char * sKey);
BOOL CreateBusLine( INITBUS & bus ) ;
char * GetBusType(BYTE byComType);
CBasePort * InitCom(BYTE byComType);
CProtocol * InitProtocol( char * pDllPath, CMethod *pMethod) ;
BOOL GetProtocolName( BYTE byType , char * sProtocol ) ;
BOOL InitComThread( ) ;
void* ThreadProc(void *pProtObj);
BOOL OpenPort(const PBUSMANAGER pBus);
void Asleep( DWORD dwMilliSecd );
BOOL AddPortOtherPara( PBUSDATA pBusData , CProfile &profile, char * busString , BYTE byNo) ;
BOOL AddNetPara( PBUSDATA pBusData , CProfile &profile , BYTE byNo) ;
BOOL GetBusProfileString( CProfile &Profile , char * sSect , char * sKey , char * sTemp , int &size );
BOOL ReadNtpServerPara(CProfile & profile);
BOOL UpdateNtpServerCfg(char * szNetMask_val, char * szIPField_val);
void printPromptInfo();
BOOL ReadFtpServerPara(CProfile & profile);
BOOL ReadBusPara(CProfile & Profile, INITBUS &bus);
void PrintConfigMsg(BYTE lineNo, PBUSDATA pBusData);
BOOL SetLocalNetPara( CProfile &Profile ) ;
BOOL ProcessRealData( PBUSMANAGER pbus , int &index , int size , PBUSMSG pBusMsg = NULL );
void setComState(CProtocol * pProto, CBasePort * pPort);
void setDevComState(CProtocol * pProto, BOOL state);
BOOL ProcessSpecialMsg(PBUSMSG BusMsg, PBUSMANAGER Bus, int size);
void SendPacketloss( PBUSMANAGER pbus ) ;
BOOL SetSerialPortMode( BYTE byComType , DWORD nPort ) ;
void ScanServerOnLine( ) ;
void* ThreadScanSever(void *pProtObj );
BOOL InitDevState( WORD wDevCount ) ;
bool checkRegisterFile();
bool modifyCode(char * szUniqueCode);
#endif // MAIN_H_INCLUDED
