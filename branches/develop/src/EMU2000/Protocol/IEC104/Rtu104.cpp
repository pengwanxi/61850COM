/**********************************************************************
  Rtu104.cpp : implementation file For IEC104 2002 on Linux
  Copyright (C): 2013 by houpeng
 ***********************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <termios.h>
#include <ctype.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/param.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <signal.h>
#include <errno.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netdb.h>

#include "Rtu104.h"
#include "../../share/TcpPort.h"
#include "../../share/msgdef.h"
#include "../../share/gDataType.h"
#include "../../BayLayer/CPublicMethod.h"

/*****************************************************************************/
#define  DATA_OK_TOUT   240  //60S
#define  ERROR_CONST    120  //30S
#define  FRAME_CONST    2    //0.5S

#define  SEND_TOUT     60  //t1=15S
#define  RECV_TOUT     40  //t2=10S
#define  IDLE_TOUT     80  //t3=20S

#define CMD_SST_BIT     0x0004  //同步时钟
#define CMD_COA_BIT     0x0020  //遥控选择
#define CMD_COE_BIT     0x0040  //遥控执行
#define CMD_COC_BIT     0x0080  //遥控撤消
#define CMD_START_BIT   0x0100  //启动发送
#define CMD_STOP_BIT    0x0200  //停止发送
#define CMD_TEST_BIT    0x0400  //测试帧确认
#define  CMD_TEST_SEND	0x0001  //发送测试帧
#define CMD_UFACK_BIT   0x0800  //发送接收序号确认帧
#define CMD_IGI_BIT     0x1000  //总召唤结束
#define CMD_RELAY_BIT   0x00E0  //遥控操作

//by zhg
#define IEC104_W			8		//w
#define IEC104_Q			12		//k
#define MAXMUM_NUM	0x8000
#define IEC104_T1			15 //S
#define IEC104_T2			10 //S
#define IEC104_T3			20 //S
#define  IEC104_T4			30 //S
#define IEC104_YKTIME	5 //S

#define IEC104_START_T1		0x01
#define IEC104_START_T2		0x02
#define IEC104_START_T3		0x04
#define IEC104_START_T4		0x08
#define IEC104_YK_STARTTIME		 0x10

#define IEC104_END_T1		  0xFE
#define IEC104_END_T2		  0xFD
#define IEC104_END_T3		  0xFB
#define IEC104_END_T4		  0xF7
#define IEC104_YK_ENDTIME		 0xEF

#define CMD_NULL						 0
#define CMD_TIME_CONFIRM		1 //对时确认
#define CMD_TIME_END				 2 //对时结束
#define CMD_TOTAL_CONFIRM	  3 //总召确认
#define CMD_YM_CONFIRM			 4 //遥脉确认
#define CMD_YM_END					  5	//遥脉召唤结束
#define CMD_YM_ERROR				6 //召唤遥脉数据出错
#define CMD_YK_ERROR				7 //发送遥控否定认可
#define CMD_DZ_WRITE_CONFIRM		8 //写定值确认
#define CMD_DZ_READ_CONFIRM   9 //读定值确认

#define YX_START_ADDR	0x0001
#define YC_START_ADDR	0x4001
#define DO_START_ADDR	0x6001
#define PA_START_ADDR	0x6401
#define COM_STATE_ADDR 50000

#define	Increase_Addr(addr)	(addr+1)%RTU104_RX_BUF_SIZE
#define	Decrease_Addr(addr)	(addr-1+RTU104_RX_BUF_SIZE)%RTU104_RX_BUF_SIZE


/*in main.cpp*/
// float CalcAIRipeVal(BYTE wStn, WORD wPnt, float nVal)
// {
// float fVal = 0;
// const ANALOGITEM *pItem = Get_RTDB_Analog(wStn, wPnt);
// if( pItem )
// {
// fVal = nVal * pItem->fRatio + pItem->fOffset;
// }
// return fVal;
// }

/*in librtdb.so*/
extern "C" int  SetCurrentTime( REALTIME *pRealTime );
extern "C" void GetCurrentTime( REALTIME *pRealTime );

/*****************************************************************************/
#ifdef	__cplusplus
extern "C" {/*{{{*/
#endif	/* __cplusplus */

	/*in main.cpp*/
	void  OutBusDebug(BYTE byBusNo, BYTE *buf, int len, int flag);
	void  OutPromptText(char *lpszText);
	void  LogPromptText(const char *fmt, ...);
	void  OutMessageText(char *szSrc, unsigned char *pData, int nLen);

	void* Rtu104Task(void *pRtuObj)
	{
		((CRtu104 *)pRtuObj)->TaskProcHandle();
		pthread_exit(0);
		return NULL;
	}

#ifdef	__cplusplus
}/*}}}*/
#endif	/* __cplusplus */

/*****************************************************************************
 * CRtu104
 */
CRtu104::CRtu104()
{/*{{{*/
	m_byProID  = 4;
	m_byEnable = 0;
	m_wDeadVal = 3;
	m_nTXCount = 0;
	m_nRXCount = 0;
	m_RQ_WR_P  = 0;
	m_RQ_RD_P  = 0;

	m_bDataInit = TRUE;
	m_bStartBit = FALSE;
	m_bAllData	= FALSE;
	m_wCommand  = 0;
	m_wStatus   = 0;
	m_bySending = 0;

	m_wNumSend  = 0;
	m_wNumRecv  = 0;
	m_wNumRecBK = 0;
	m_wNumAck  = 0;

	m_wDataIndex = 0 ;
	m_byDataStyle = 0 ;

	m_wErrTimer   = 0;
	m_wTimerCount = 0;
	m_wSendTimer  = 0;
	m_wRecvTimer  = 0;
	m_wIdleTimer  = 0;
	m_wDRFTimer   = 0;
	m_wPARTimer   = 0;
	m_byTimeFlag = 0 ;
	m_t1 = 0 ;
	m_t2 = 0 ;
	m_t3 = 0 ;

	m_byErrCode = 0;
	m_wResendNum = 0;
	m_byToutNum = 0;

	QueryAllDevStatus = TRUE;			//by cyz!
	m_byDzOpt = -1;
	m_byDzNum = 0;
	//     memset((void*)&m_wAIBuf[0], 0, sizeof(WORD)*RTU104MAX_AI_LEN);
	//     memset((void*)&m_dwPIBuf[0], 0, sizeof(DWORD)*RTU104MAX_PI_LEN);
	// 		memset( ( void * )&m_byDIbuf[ 0 ] , 0 , sizeof( BYTE ) * RTU104MAX_DI_LEN ) ;

	InitBuffer<float>( m_wAIBuf , RTU104MAX_AI_LEN ) ;
	InitBuffer<QWORD>( m_dwPIBuf , RTU104MAX_PI_LEN );
	InitBuffer<BYTE>( m_byDIbuf , RTU104MAX_DI_LEN ) ;
}/*}}}*/

CRtu104::~CRtu104()
{/*{{{*/
	m_bTaskRun = FALSE;
}/*}}}*/

template< typename Type >void CRtu104::InitBuffer( Type *tData , int size )
{/*{{{*/
	if( !tData )
		return ;

	for( int i = 0 ; i < size ; i++ )
		tData[ i ] = 0 ;
}/*}}}*/

BOOL CRtu104::InitRtuBase( )
{/*{{{*/
	UINT uPort;
	BOOL bOk = FALSE;
	char szCtrl[32];

	//OutPromptText("****CRtu104.Init()****");

	CBasePort::GetCommAttrib(m_ComCtrl1, szCtrl, uPort);

	m_wPortNum = (WORD)uPort ;

	//获取转发序号
	CreateTransTab();

	//从内存数据库中--获取转发表默认数据
	m_pMethod->ReadAllYcData(&m_wAIBuf[0]);
	m_pMethod->ReadAllYmData(&m_dwPIBuf[0]);
	m_pMethod->ReadAllYxData( &m_byDIbuf[ 0 ] ) ;

	m_bTaskRun = TRUE;
	return bOk;
}/*}}}*/

void CRtu104::TimerProc()
{/*{{{*/

	//从内存中读取变化遥信和遥测数据
	ReadChangData();

	//读所有设备状态变化者写入deque!
	//	ReadChangeStatus(vec_stat, deque_stat);					//by cyz!
	ReadChangeStatus(map_stat, deque_stat);					//by cyz!
	//处理IEC104 T1 , T2 , T3, T4, YK_TIME
	ProcessIEC104Time( ) ;
}/*}}}*/

void CRtu104::ProcessIEC104Time( )
{/*{{{*/
	BYTE byTimeFlag = GetTimeFlag() ;
	time_t timeTemp ;
	time( &timeTemp ) ;
	if( ( byTimeFlag & IEC104_START_T1 ) == IEC104_START_T1 )
	{
		if( difftime( timeTemp , m_t1 ) > IEC104_T1 )
		{
			//处理IEC104所有状态
			ReSetState( ) ;
			//关闭网络连接
			if( m_pMethod )
				m_pMethod->CloseSocket( m_byLineNo ) ;
		}
	}

	if( ( byTimeFlag & IEC104_START_T2 ) == IEC104_START_T2 )
	{
		if( difftime( timeTemp , m_t2 ) > IEC104_T2 )
		{
			//发送S帧计数
			m_wCommand |= CMD_UFACK_BIT ;
		}
	}

	if( ( byTimeFlag & IEC104_START_T3 ) == IEC104_START_T3 )
	{

		if( difftime( timeTemp , m_t3 ) > IEC104_T3 )
		{
			//发送测试帧计数
			m_wCommand |= CMD_TEST_SEND ;
		}
	}

	if( ( byTimeFlag & IEC104_START_T4 ) == IEC104_START_T4 )
	{
		if( difftime( timeTemp , m_t4 ) > IEC104_T4 )
		{
			//处理IEC104所有状态
			ReSetState( ) ;
			//关闭网络连接
			if( m_pMethod )
				m_pMethod->CloseSocket( m_byLineNo ) ;
		}
	}

	if( ( byTimeFlag & IEC104_YK_STARTTIME ) == IEC104_YK_STARTTIME )
	{
		time( &timeTemp) ;
		if( difftime( timeTemp , m_YKTime ) > IEC104_YKTIME )
		{
			BYTE byAction = 0 ;
			WORD  wRelayNum = 0 ;
			BYTE byActNum = 0 ;
			BYTE byActMark = 0 ;
			WORD wReason = 0 ;
			GetYkData( byAction , wRelayNum, byActNum , byActMark , wReason ) ;
			YK_ErrorProcess( byAction , wRelayNum , byActNum , byActMark , wReason ) ;
			SetTTimer( IEC104_YKTIME , IEC104_YK_ENDTIME ) ;
		}
	}
}/*}}}*/

BYTE CRtu104::CalBch(BYTE* pBuf, int len)
{/*{{{*/
	int	  i;
	BYTE  byCheckCode=0;
	for(i=0; i<len; i++) byCheckCode += pBuf[i];
	return byCheckCode;
}/*}}}*/

int CRtu104::GetCommObjProp(COMMOBJ_PROP* pObjProp)
{/*{{{*/
	pObjProp->byStyle  = 2;
	pObjProp->byObjNum = LOBYTE(m_wObjNum);
	sprintf(pObjProp->szObjName, m_szObjName);
	sprintf(pObjProp->szChannel, m_ComCtrl1);
	pObjProp->wPortNum = m_wPortNum;
	pObjProp->wStatus  = 0;
	if( IsPortValid() )
		pObjProp->wStatus = 1;
	return 0;
}/*}}}*/

///////////////////////////////////////////////////////////////////////////////
int CRtu104::GetRealVal(BYTE byType, WORD wPnt, void *v)
{/*{{{*/
	WORD  wValue = 0 ;
	switch(byType)
	{
	case 0:
		if(wPnt>=RTU104MAX_AI_LEN) return -2;
		memcpy(v, &m_wAIBuf[wPnt], sizeof(float));
		break;
	case 1:
		{
			if(wPnt>=RTU104MAX_DI_LEN)
				return -2;

			if( m_byDIbuf[ wPnt ] ==0 )
				wValue = 0;
			else
				wValue = 1;

			memcpy(v, &wValue, sizeof(WORD));
		}
		break;
	case 2:
		if(wPnt>=RTU104MAX_PI_LEN) return -2;
		memcpy(v, &m_dwPIBuf[wPnt], sizeof(QWORD));
		break;
	default:
		return -1;
	}
	return 0;
}/*}}}*/

/*{{{*/
/*
   BOOL WriteCIVal( WORD wSerialNo , WORD wPnt, float fVal)
   {
   if(m_pwCITrans==NULL) return;
   if(wPnt>= MAPMAX_CI_LEN) return;
   WORD wNum = m_pwCITrans[wPnt];
   if(wNum>GetPntSum( YC_SUM )) return;
   if(wNum>0 && wNum<=RTU104MAX_AI_LEN)
   {
   WORD wVal = (WORD)fVal;
   if(wVal != m_wAIBuf[wNum-1])
   {
   m_wAIBuf[wNum-1] = wVal;
   if(m_bDataInit)
   AddAnalogEvt( wSerialNo ,wNum , wVal);
   }
   }
   }
   */
/*}}}*/

/*
 * ------------------------------------------------------------------
 *  para0:顺序号，从零开始
 *  para1:转发通道点号
 *  para2:转发值
 * ------------------------------------------------------------------
 */
BOOL CRtu104::WriteAIVal(WORD wSerialNo ,WORD wPnt, float fVal)
{/*{{{*/
	if(m_pwAITrans==NULL) return FALSE;
	WORD wNum = m_pwAITrans[wPnt];				//wNum:转发配置“序号”
	if(wNum> m_wAISum ) return FALSE;
	if(wNum<RTU104MAX_AI_LEN)//mengqp 将<=改为< 否则m_wAIBuf[4096]越界
	{
		float nDelt = fVal - m_wAIBuf[wNum];
		if(abs(( int )nDelt)>=m_wDeadVal)				//- by cyz!
	//	if(llabs((long long int)nDelt)>=m_wDeadVal)		//+ by cyz!
		{
			m_wAIBuf[wNum] = fVal;
			if(m_bDataInit){
				if(wSerialNo >= m_pMethod->GetGatherDevCount())		//+2 by cyz!
					return FALSE;
				AddAnalogEvt( wSerialNo , wNum, fVal );
			}


// 			if (fVal != 0)
// 				return TRUE;
// 
// 			REALTIME CurrTime;
// 			GetCurrentTime(&CurrTime);
// 			char stime[255] = { 0 };
// 			sprintf(stime, "%04d-%02d-%02d %02d:%02d:%02d", CurrTime.wYear,
// 				CurrTime.wMonth, CurrTime.wDay, CurrTime.wHour, CurrTime.wMinute,
// 				CurrTime.wSecond);
// 
// 			char sbuf[300] = { 0 };
// 			sprintf(sbuf, "IEC104 %s,serialno = %d , dataPos=%d,%f\n", stime, wSerialNo, wNum, fVal);
// 			writeLog(sbuf, strlen(sbuf));
// 			printf(sbuf);
// 			printf("\n");
		}
	}
	return TRUE ;
}/*}}}*/


void CRtu104::writeLog(char * pContent, int len)
{
	int ret = access("/mynand/log", F_OK);
	if (ret == -1)
	{
		mkdir("/mynand/log", 0755);
	}
	FILE * m_logFile = NULL;
	char fileName[255] = { 0 };
	sprintf(fileName, "/mynand/log/%d_%d.txt", m_byLineNo, m_wDevAddr);
	m_logFile = fopen(fileName, "a+");

	fseek(m_logFile, 0, SEEK_END);
	fwrite(pContent, len, 1, m_logFile);

	fclose(m_logFile);
}

BOOL CRtu104::WriteDIVal(WORD wSerialNo ,WORD wPnt, WORD wVal)
{/*{{{*/
	if(m_pwDITrans==NULL) return FALSE;
	WORD wNum = m_pwDITrans[wPnt] & 0x7fff;
	if(wNum>m_wDISum ) return FALSE;
	if( wNum<RTU104MAX_DI_LEN)//mengqp 将<= 改为<
	{
		if( m_byDIbuf[ wNum ] != wVal )
		{
			m_byDIbuf[ wNum ] = wVal ;
			if(m_bDataInit)
				AddDigitalEvt( wSerialNo , wNum, wVal);
		}
	}
	return TRUE ;
}/*}}}*/

BOOL CRtu104::WritePIVal(WORD wSerialNo ,WORD wPnt, QWORD dwVal)
{/*{{{*/
	if(m_pwPITrans==NULL) return FALSE;
	WORD wNum = m_pwPITrans[wPnt];
	if(wNum> m_wPISum ) return FALSE;
	if(wNum<RTU104MAX_PI_LEN)//mengqp 将<= 改为<
	{
		m_dwPIBuf[wNum] = dwVal;
	}
	return TRUE;
}/*}}}*/

BOOL CRtu104::WriteSOEInfo( WORD wSerialNo ,WORD wPnt, WORD wVal, LONG lTime, WORD wMiSecond)
{/*{{{*/
	if(m_pwDITrans==NULL) return FALSE;
	WORD wNum = m_pwDITrans[wPnt] & 0x7fff;
	if(wNum>= m_wDISum ) return FALSE;
	if(wNum<RTU104MAX_DI_LEN)
	{
		AddSOEInfo(wSerialNo , wNum, wVal, lTime, wMiSecond);
	}
	return TRUE ;
}/*}}}*/

void CRtu104::RelayEchoProc(BYTE byCommand, WORD wIndex, BYTE byResult)
{/*{{{*/
	BYTE byAction = 0 ;
	WORD  wRelayNum = 0 ;
	BYTE byActNum = 0 ;
	BYTE byActMark = 0 ;
	WORD wSelectTime = 0 ;
	GetYkData( byAction , wRelayNum, byActNum , byActMark , wSelectTime ) ;

	WORD wCtrlNum;
	if(byAction == 0) return;
	if(m_pwDOTrans==NULL) return;
	wCtrlNum = m_pwDOTrans[wIndex] ;
	printf("RelayProc\n");
	switch(byCommand)
	{
	case YK_SEL_RTN:
		m_wYkFlag = YK_SEL_RTN;
		if( wCtrlNum == wRelayNum )
		{
			m_wCommand |= CMD_COA_BIT;
		}
		break;
	case YK_EXCT_RTN:
		m_wYkFlag = YK_EXCT_RTN;
		if( wCtrlNum == wRelayNum )
		{
			m_wCommand |= CMD_COA_BIT;
		}
		break;
	case YK_CANCEL_RTN:
		m_wYkFlag = YK_CANCEL_RTN ;
		if( wCtrlNum == wRelayNum )
		{
			m_wCommand |= CMD_COA_BIT ;
		}
		break;
	case 0x32:
		break;
	}
}/*}}}*/

int CRtu104::TransMessage( BYTE byType, BYTE* pBuf, int nLen )
{/*{{{*/
	return 0;
}/*}}}*/

/*****************************************************************************/
int CRtu104::AckFrame(void)
{/*{{{*/
	WORD wNS, wNR;

	GetSendRecvNo( wNS , wNR ) ;

	m_pTX_Buf[0] = 0x68;
	m_pTX_Buf[1] = 0x04;
	m_pTX_Buf[2] = 0x01;
	m_pTX_Buf[3] = 0x00;
	m_pTX_Buf[4] = LOBYTE(wNR);
	m_pTX_Buf[5] = HIBYTE(wNR);
	return 6;
}/*}}}*/

int CRtu104::StartDataAck(void)
{/*{{{*/
	m_pTX_Buf[0] = 0x68;
	m_pTX_Buf[1] = 0x04;
	m_pTX_Buf[2] = 0x03+0x08;
	m_pTX_Buf[3] = 0x00;
	m_pTX_Buf[4] = 0x00;
	m_pTX_Buf[5] = 0x00;
	return 6;
}/*}}}*/

int CRtu104::StopDataAck(void)
{/*{{{*/
	m_pTX_Buf[0] = 0x68;
	m_pTX_Buf[1] = 0x04;
	m_pTX_Buf[2] = 0x03+0x20;
	m_pTX_Buf[3] = 0x00;
	m_pTX_Buf[4] = 0x00;
	m_pTX_Buf[5] = 0x00;
	return 6;
}/*}}}*/

int CRtu104::TestFrameAck(void)
{/*{{{*/
	m_pTX_Buf[0] = 0x68;
	m_pTX_Buf[1] = 0x04;
	m_pTX_Buf[2] = 0x03+0x80;
	m_pTX_Buf[3] = 0x00;
	m_pTX_Buf[4] = 0x00;
	m_pTX_Buf[5] = 0x00;
	return 6;
}/*}}}*/

int CRtu104::TestSend( )
{/*{{{*/
	m_pTX_Buf[0] = 0x68;
	m_pTX_Buf[1] = 0x04;
	m_pTX_Buf[2] = 0x03+0x40;
	m_pTX_Buf[3] = 0x00;
	m_pTX_Buf[4] = 0x00;
	m_pTX_Buf[5] = 0x00;
	return 6;
}/*}}}*/

int CRtu104::AllDataEcho(WORD wUnitAddr, BYTE byReason)
{/*{{{*/
	WORD wNS, wNR;
	BYTE byLen;
	BYTE *pTXBuf = m_pTX_Buf;

	GetSendRecvNo( wNS , wNR ) ;

	pTXBuf[0] = 0x68;				//启动字符
	pTXBuf[2] = LOBYTE(wNS);
	pTXBuf[3] = HIBYTE(wNS);
	pTXBuf[4] = LOBYTE(wNR);
	pTXBuf[5] = HIBYTE(wNR);
	pTXBuf[6] = 0x64;				//100 类型标识TYP
	pTXBuf[7] = 0x01;				//	 可变结构限定词VSQ
	pTXBuf[8] = byReason;			//   传送原因COT 2 byte
	pTXBuf[9] = 0;					//7-激活确认  9-停止激活确认  10-激活结束
	pTXBuf[10] = LOBYTE(wUnitAddr);	//数据单元地址 2 byte
	pTXBuf[11] = HIBYTE(wUnitAddr);
	pTXBuf[12] = 0;					//信息体地址 3 byte
	pTXBuf[13] = 0;
	pTXBuf[14] = 0;
	pTXBuf[15] = 0x14;
	byLen = 16;
	pTXBuf[1] = byLen-2;			//长度L
	return byLen;
}/*}}}*/

int CRtu104::ReqPulseEcho(WORD wUnitAddr, BYTE byReason, BYTE byQCC)
{/*{{{*/
	WORD wNS, wNR;
	BYTE byLen;
	BYTE *pTXBuf = m_pTX_Buf;

	GetSendRecvNo( wNS , wNR ) ;
	pTXBuf[0] = 0x68;				//启动字符
	pTXBuf[2] = LOBYTE(wNS);
	pTXBuf[3] = HIBYTE(wNS);
	pTXBuf[4] = LOBYTE(wNR);
	pTXBuf[5] = HIBYTE(wNR);
	pTXBuf[6] = 0x65;				//101 类型标识TYP
	pTXBuf[7] = 0x01;				//	 可变结构限定词VSQ
	pTXBuf[8] = byReason;			//   传送原因COT 2 byte
	pTXBuf[9] = 0;					//7-激活确认  10-激活结束
	pTXBuf[10] = LOBYTE(wUnitAddr);	//数据单元地址 2 byte
	pTXBuf[11] = HIBYTE(wUnitAddr);
	pTXBuf[12] = 0;					//信息体地址 3 byte
	pTXBuf[13] = 0;
	pTXBuf[14] = 0;
	pTXBuf[15] = byQCC;
	byLen = 16;
	pTXBuf[1] = byLen-2;			//长度L
	return byLen;
}/*}}}*/

int CRtu104::LoadAIEFrame21(WORD wUnitAddr)
{/*{{{*/
	WORD wNS, wNR, wPnt ;
	float fVal;
	BYTE byLen, byCount;
	int  i, nSize;
	BYTE *pTXBuf = m_pTX_Buf;
	WORD wSerialNo = 0 ;

	GetSendRecvNo( wNS , wNR ) ;
	pTXBuf[0] = 0x68;				//启动字符
	pTXBuf[2] = LOBYTE(wNS);
	pTXBuf[3] = HIBYTE(wNS);
	pTXBuf[4] = LOBYTE(wNR);
	pTXBuf[5] = HIBYTE(wNR);
	pTXBuf[6] = 0x15;				//21 类型标识TYP
	pTXBuf[8] = 0x03;				//3  传送原因COT(突发) 2 byte
	pTXBuf[9] = 0;
	pTXBuf[10] = LOBYTE(wUnitAddr);	//数据单元地址 2 byte
	pTXBuf[11] = HIBYTE(wUnitAddr);
	byLen = 12;

	byCount = 0;
	nSize = m_dwAIEQueue.size();
	for(i=0; i<nSize; i++)
	{
		if(!GetAnalogEvt(wSerialNo , wPnt, fVal)) break;
		WORD wVal = static_cast<WORD>( fVal ) ;

		wPnt += YC_START_ADDR;
		pTXBuf[byLen]   = LOBYTE(wPnt);
		pTXBuf[byLen+1] = HIBYTE(wPnt);
		pTXBuf[byLen+2] = 0;
		pTXBuf[byLen+3] = LOBYTE(wVal);
		pTXBuf[byLen+4] = HIBYTE(wVal);
		byCount++;
		byLen += 5;
		if( byLen >= 250 ) break;
	}
	pTXBuf[7] = byCount;			//可变结构限定词VSQ
	pTXBuf[1] = byLen-2;			//长度L
	return byLen;
}/*}}}*/

int CRtu104::LoadAIEFrame13(WORD wUnitAddr)
{/*{{{*/
	WORD wNS, wNR;
	WORD wPnt, wNum ;
	float fGetVal = 0.0f ;
	BYTE byLen, byCount;
	int  i, nSize;
	float fVal;
	WORD wSerialNo = 0 ;
	BYTE *pTXBuf = m_pTX_Buf;

	GetSendRecvNo( wNS , wNR ) ;
	pTXBuf[0] = 0x68;				//启动字符
	pTXBuf[2] = LOBYTE(wNS);
	pTXBuf[3] = HIBYTE(wNS);
	pTXBuf[4] = LOBYTE(wNR);
	pTXBuf[5] = HIBYTE(wNR);
	pTXBuf[6] = 13;					//13 类型标识TYP
	pTXBuf[8] = 0x03;				//3  传送原因COT(突发) 2 byte
	pTXBuf[9] = 0;
	pTXBuf[10] = LOBYTE(wUnitAddr);	//数据单元地址 2 byte
	pTXBuf[11] = HIBYTE(wUnitAddr);
	byLen = 12;

	byCount = 0;
	nSize = m_dwAIEQueue.size();		//- by cyz!
	//nSize = counter;					//+ by cyz!
	//为了兰大二院添加字段，表示变化遥测的数目，变位遥测保存到双向链表中。数目由counter记录,定义在Rtu.h中
	//printf("********	file:%s counter:%d	********\n", __FILE__, counter);
	for(i=0; i<nSize; i++)
	{
		if(!GetAnalogEvt( wSerialNo , wNum, fGetVal))
			break;

		BYTE byBuffer[ 4 ] ;
		BYTE byBuffer1[ 4 ] ;

		fVal = fGetVal;
		if( m_pAIMapTab[wNum].wStn>0 &&
				m_pAIMapTab[wNum].wPntNum>0 )
		{
			fVal = CalcAIRipeVal(m_pAIMapTab[wNum].wStn,				//为何fVal要求取两次!
					m_pAIMapTab[wNum].wPntNum,
					fGetVal);
			memcpy( byBuffer , &fVal , 4 ) ;
			GlobalCopyByEndian( byBuffer1, byBuffer, 4);
			// byBuffer1[ 0 ] = byBuffer[ 3 ] ;
			// byBuffer1[ 1 ] = byBuffer[ 2 ] ;
			// byBuffer1[ 2 ] = byBuffer[ 1 ] ;
			// byBuffer1[ 3 ] = byBuffer[ 0 ] ;
		//printf("**** fVal:%f ****\n", fVal);
		//printf("[0]:%02x ", byBuffer1[0]);
		//printf("[1]:%02x ", byBuffer1[1]);
		//printf("[2]:%02x ", byBuffer1[2]);
		//printf("[3]:%02x\n", byBuffer1[3]);
		}
		wPnt = YC_START_ADDR + wNum;
		pTXBuf[byLen]   = LOBYTE(wPnt);
		pTXBuf[byLen+1] = HIBYTE(wPnt);
		pTXBuf[byLen+2] = 0;
		memcpy(&pTXBuf[byLen+3], &byBuffer1, 4);

		//处理品质描述词
		BOOL bDevState = FALSE ;
		BYTE bySiq = 0 ;

		bDevState = m_pMethod->GetDevCommState( wSerialNo ) ;
		if( bDevState == COM_DEV_ABNORMAL )
			bySiq |= 0xC0 ;

		pTXBuf[byLen+7] = bySiq ;
		byCount++;
		byLen += 8;
		if( byLen > 245 ) break;
	}
	pTXBuf[7] = byCount;			//可变结构限定词VSQ
	pTXBuf[1] = byLen-2;			//长度L
	return byLen;
}/*}}}*/

int CRtu104::LoadDIEFrame(WORD wUnitAddr)
{/*{{{*/
	WORD wNS, wNR, wPnt, wVal;
	BYTE byLen, byCount;
	int  i, nSize;
	BYTE bySiq = 0 ;
	WORD wSerialNo = 0 ;
	BYTE *pTXBuf = m_pTX_Buf;

	GetSendRecvNo( wNS , wNR ) ;
	pTXBuf[0] = 0x68;				//启动字符
	pTXBuf[2] = LOBYTE(wNS);
	pTXBuf[3] = HIBYTE(wNS);
	pTXBuf[4] = LOBYTE(wNR);
	pTXBuf[5] = HIBYTE(wNR);
	pTXBuf[6] = 0x01;				//1  类型标识TYP
	pTXBuf[8] = 0x03;				//3  传送原因COT(突发) 2 byte
	pTXBuf[9] = 0;
	pTXBuf[10] = LOBYTE(wUnitAddr);	//数据单元地址 2 byte
	pTXBuf[11] = HIBYTE(wUnitAddr);
	byLen = 12;

	byCount = 0;
	nSize = m_dwDIEQueue.size();
	for(i=0; i<nSize; i++)
	{
		if(!GetDigitalEvt( wSerialNo , wPnt, wVal)) break;
		wPnt += YX_START_ADDR;
		pTXBuf[byLen]   = LOBYTE(wPnt);
		pTXBuf[byLen+1] = HIBYTE(wPnt);
		pTXBuf[byLen+2] = 0;

		if ( wVal & 0x01 )
		{
			bySiq = 0x01 ;
		}
		else
			bySiq = 0 ;

		BOOL bDevState = FALSE ;
		bDevState = m_pMethod->GetDevCommState( wSerialNo ) ;
		if( bDevState == COM_DEV_ABNORMAL )
			bySiq |= 0xC0 ;

		pTXBuf[byLen+3] = bySiq ;
		byCount++;
		byLen += 4;
		if( byLen >= 251 ) break;
	}
	pTXBuf[7] = byCount;			//可变结构限定词VSQ
	pTXBuf[1] = byLen-2;			//长度L
	return byLen;
}/*}}}*/

int CRtu104::LoadSOEFrame_30 (WORD wUnitAddr)
{/*{{{*/
	WORD   wNS, wNR;
	WORD   wPnt, wVal, wTime, wMiSecond;
	struct tm  tmStruct;
	BYTE   byLen = 0 , byCount = 0 ;
	WORD wSerialNo = 0 ;
	BYTE   *pTXBuf = m_pTX_Buf;

	GetSendRecvNo( wNS , wNR ) ;
	pTXBuf[byLen++] = 0x68;				//启动字符
	pTXBuf[byLen++] = 0 ;
	pTXBuf[byLen++] = LOBYTE(wNS);
	pTXBuf[byLen++] = HIBYTE(wNS);
	pTXBuf[byLen++] = LOBYTE(wNR);
	pTXBuf[byLen++] = HIBYTE(wNR);
	pTXBuf[byLen++] = 0x1E;				//2  类型标识TYP
	pTXBuf[byLen++] = 0 ;
	pTXBuf[byLen++] = 0x03;				//3  传送原因COT(突发) 2 byte
	pTXBuf[byLen++] = 0;
	pTXBuf[byLen++] = LOBYTE(wUnitAddr);	//数据单元地址 2 byte
	pTXBuf[byLen++] = HIBYTE(wUnitAddr);

	byCount = 0;
	while(m_iSOE_rd_p != m_iSOE_wr_p)
	{
		GetSOEInfo( wSerialNo , &wPnt, &wVal, &tmStruct, &wMiSecond);

		wPnt  = wPnt + YX_START_ADDR;
		wVal  = wVal & 0x0001;

		wTime=tmStruct.tm_sec * 1000 +wMiSecond;

		pTXBuf[byLen++]   = LOBYTE(wPnt);
		pTXBuf[byLen++] = HIBYTE(wPnt);
		pTXBuf[byLen++] = 0;

		pTXBuf[byLen++] = LOBYTE(wVal);

		pTXBuf[byLen++]=LOBYTE(wTime);
		pTXBuf[byLen++]=HIBYTE(wTime);
		pTXBuf[byLen++]=tmStruct.tm_min;
		pTXBuf[byLen++]=tmStruct.tm_hour;
		pTXBuf[byLen++]=tmStruct.tm_mday;
		pTXBuf[byLen++]=tmStruct.tm_mon;
		pTXBuf[byLen++]=tmStruct.tm_year%100;

		byCount++;
		if( byLen > 246 ) break;
	}
	pTXBuf[7] = byCount;			//可变结构限定词VSQ
	pTXBuf[1] = byLen-2;			//长度L
	return byLen;
}/*}}}*/

int CRtu104::LoadSOEFrame_02(WORD wUnitAddr)
{/*{{{*/
	WORD   wNS, wNR;
	WORD   wPnt, wVal, wTime, wMiSecond;
	struct tm  tmStruct;
	BYTE   byLen, byCount;
	WORD wSerialNo = 0 ;
	BYTE   *pTXBuf = m_pTX_Buf;

	GetSendRecvNo( wNS , wNR ) ;
	pTXBuf[0] = 0x68;				//启动字符
	pTXBuf[2] = LOBYTE(wNS);
	pTXBuf[3] = HIBYTE(wNS);
	pTXBuf[4] = LOBYTE(wNR);
	pTXBuf[5] = HIBYTE(wNR);
	pTXBuf[6] = 0x02;				//2  类型标识TYP
	pTXBuf[8] = 0x03;				//3  传送原因COT(突发) 2 byte
	pTXBuf[9] = 0;
	pTXBuf[10] = LOBYTE(wUnitAddr);	//数据单元地址 2 byte
	pTXBuf[11] = HIBYTE(wUnitAddr);
	byLen = 12;

	byCount = 0;
	while(m_iSOE_rd_p != m_iSOE_wr_p)
	{
		GetSOEInfo( wSerialNo , &wPnt, &wVal, &tmStruct, &wMiSecond);
		wPnt  = wPnt + YX_START_ADDR;
		wVal  = wVal & 0x0001;
		wTime = tmStruct.tm_sec*1000 + wMiSecond;
		pTXBuf[byLen]   = LOBYTE(wPnt);
		pTXBuf[byLen+1] = HIBYTE(wPnt);
		pTXBuf[byLen+2] = 0;
		pTXBuf[byLen+3] = LOBYTE(wVal);
		pTXBuf[byLen+4] = LOBYTE(wTime);
		pTXBuf[byLen+5] = HIBYTE(wTime);
		pTXBuf[byLen+6] = (BYTE)tmStruct.tm_min;
		byCount++;
		byLen += 7;
		if( byLen > 246 ) break;
	}
	pTXBuf[7] = byCount;			//可变结构限定词VSQ
	pTXBuf[1] = byLen-2;			//长度L
	return byLen;
}/*}}}*/

int CRtu104::LoadAnalogData21(WORD wUnitAddr)
{/*{{{*/
	WORD wNS, wNR, wPnt ;
	float fVal = 0.0f;
	BYTE byLen, byCount;
	int  i;
	BYTE *pTXBuf = m_pTX_Buf;

	GetSendRecvNo( wNS , wNR ) ;
	pTXBuf[0] = 0x68;				//启动字符
	pTXBuf[2] = LOBYTE(wNS);
	pTXBuf[3] = HIBYTE(wNS);
	pTXBuf[4] = LOBYTE(wNR);
	pTXBuf[5] = HIBYTE(wNR);
	pTXBuf[6] = 0x15;				//21 类型标识TYP
	pTXBuf[8] = 0x14;				//20  传送原因COT(响应总召唤) 2 byte
	pTXBuf[9] = 0;
	pTXBuf[10] = LOBYTE(wUnitAddr);	//数据单元地址 2 byte
	pTXBuf[11] = HIBYTE(wUnitAddr);
	wPnt = YC_START_ADDR + m_wDataIndex;
	pTXBuf[12] = LOBYTE(wPnt);
	pTXBuf[13] = HIBYTE(wPnt);
	pTXBuf[14] = 0;
	byLen = 15;

	byCount = 0;
	for(i=m_wDataIndex; i< m_wAISum ; i++)
	{
		fVal = m_wAIBuf[i];
		if( m_pAIMapTab[i].wStn>0 && m_pAIMapTab[i].wPntNum>0 )
			fVal = CalcAIRipeVal(m_pAIMapTab[i].wStn, m_pAIMapTab[i].wPntNum, m_wAIBuf[i]);
		WORD wVal = ( WORD )fVal ;
		pTXBuf[byLen]   = LOBYTE(wVal);
		pTXBuf[byLen+1] = HIBYTE(wVal);
		byCount++;
		byLen += 2;
		if( byLen >= 253 ) break;
	}
	pTXBuf[7] = 0x80 | byCount;		//可变结构限定词VSQ
	pTXBuf[1] = byLen-2;			//长度L

	m_wDataIndex += byCount;
	if(m_wDataIndex>=m_wAISum )
	{
		SetTransIndex( 0 , 0x20 ) ;
	}
	return byLen;
}/*}}}*/

int CRtu104::LoadAnalogData13(WORD wUnitAddr)				//后缀13表示类型标识!
{/*{{{*/
	WORD wNS, wNR, wPnt;
	BYTE byLen, byCount;
	int   i;
	float fVal;
	BYTE *pTXBuf = m_pTX_Buf;

	GetSendRecvNo( wNS , wNR ) ;
	pTXBuf[0] = 0x68;				//启动字符
	pTXBuf[2] = LOBYTE(wNS);
	pTXBuf[3] = HIBYTE(wNS);
	pTXBuf[4] = LOBYTE(wNR);
	pTXBuf[5] = HIBYTE(wNR);
	pTXBuf[6] = 13;					//13 类型标识TYP
	pTXBuf[8] = 0x14;				//20 传送原因COT(响应总召唤) 2 byte
	pTXBuf[9] = 0;
	pTXBuf[10] = LOBYTE(wUnitAddr);	//数据单元地址 2 byte
	pTXBuf[11] = HIBYTE(wUnitAddr);
	wPnt = YC_START_ADDR + m_wDataIndex;
	pTXBuf[12] = LOBYTE(wPnt);
	pTXBuf[13] = HIBYTE(wPnt);
	pTXBuf[14] = 0;
	byLen = 15;

	byCount = 0;
	WORD wAISum = m_wAISum;
	for(i=m_wDataIndex; i<wAISum; i++)
	{
		fVal = m_wAIBuf[i];
		if( m_pAIMapTab[i].wStn>0 && m_pAIMapTab[i].wPntNum>0 )
			fVal = CalcAIRipeVal(m_pAIMapTab[i].wStn, m_pAIMapTab[i].wPntNum, m_wAIBuf[i]);				//系数和补偿转换!

		//处理fVal顺序 linux和window在传送浮点数时，高低字节不对
		BYTE byBuffer[ 4 ] ;
		BYTE byBuffer1[ 4 ] ;
		memcpy( byBuffer , &fVal , 4 ) ;
		GlobalCopyByEndian( byBuffer1, byBuffer, 4);
		// byBuffer1[ 0 ] = byBuffer[ 3 ] ;
		// byBuffer1[ 1 ] = byBuffer[ 2 ] ;
		// byBuffer1[ 2 ] = byBuffer[ 1 ] ;
		// byBuffer1[ 3 ] = byBuffer[ 0 ] ;

		memcpy(&pTXBuf[byLen], byBuffer1, 4);

		//处理品质描述词
		WORD wSerialNo = 0xFFFF ;
		BYTE bySiq = 0 ;
		wSerialNo = GetSerialNoFromTrans( YC_TRANSTOSERIALNO , i ) ;
		BOOL bDevState = FALSE ;
		bDevState = m_pMethod->GetDevCommState( wSerialNo ) ;
		if( bDevState == COM_DEV_ABNORMAL )
			bySiq |= 0xC0 ;

		pTXBuf[byLen+4] = bySiq ;
		byCount++;
		byLen += 5;
		if( byLen > 248 ) break;
	}
	pTXBuf[7] = 0x80 | byCount;		//可变结构限定词VSQ
	pTXBuf[1] = byLen-2;			//长度L

	m_wDataIndex += byCount;
	if(m_wDataIndex>= wAISum)
	{
		SetTransIndex( 0 , 0x20 ) ;
	}
	return byLen;
}/*}}}*/

int CRtu104::LoadAnalogGroup(BYTE byGroup, WORD wUnitAddr, BYTE byReason)
{/*{{{*/
	WORD wNS, wNR, wIndex, wPnt, wVal;
	BYTE byLen, byCount;
	int  i;
	float fVal = 0.0f ;
	BYTE *pTXBuf = m_pTX_Buf;

	GetSendRecvNo( wNS , wNR ) ;
	pTXBuf[0] = 0x68;				//启动字符
	pTXBuf[2] = LOBYTE(wNS);
	pTXBuf[3] = HIBYTE(wNS);
	pTXBuf[4] = LOBYTE(wNR);
	pTXBuf[5] = HIBYTE(wNR);
	pTXBuf[6] = 0x15;				//类型标识TYP=21
	pTXBuf[8] = byReason;			//传送原因COT(响应分组召唤29-36) 2 byte
	pTXBuf[9] = 0;
	pTXBuf[10] = LOBYTE(wUnitAddr);	//数据单元地址 2 byte
	pTXBuf[11] = HIBYTE(wUnitAddr);

	wIndex = byGroup * 128;
	wPnt = YC_START_ADDR + wIndex;
	pTXBuf[12] = LOBYTE(wPnt);
	pTXBuf[13] = HIBYTE(wPnt);
	pTXBuf[14] = 0;
	byLen = 15;

	byCount = 0;
	WORD wAISum = GetPntSum( YC_SUM ) ;
	for(i=wIndex; i< wAISum; i++)
	{
		fVal = m_wAIBuf[i];
		if( m_pAIMapTab[i].wStn>0 && m_pAIMapTab[i].wPntNum>0 )
			fVal = CalcAIRipeVal(m_pAIMapTab[i].wStn, m_pAIMapTab[i].wPntNum, m_wAIBuf[i]);
		wVal = ( WORD )fVal ;
		pTXBuf[byLen]   = LOBYTE(wVal);
		pTXBuf[byLen+1] = HIBYTE(wVal);
		byCount++;
		byLen += 2;
		if( byLen >= 253 ) break;
	}
	pTXBuf[7] = 0x80 | byCount;		//可变结构限定词VSQ
	pTXBuf[1] = byLen-2;			//长度L
	return byLen;
}/*}}}*/

int CRtu104::LoadDigitalData(WORD wUnitAddr)
{/*{{{*/
	WORD wNS, wNR, wPnt, wVal;
	BYTE byLen, byCount;
	int  i;
	BYTE *pTXBuf = m_pTX_Buf;

	GetSendRecvNo( wNS , wNR ) ;
	pTXBuf[0] = 0x68;				//启动字符
	pTXBuf[2] = LOBYTE(wNS);
	pTXBuf[3] = HIBYTE(wNS);
	pTXBuf[4] = LOBYTE(wNR);
	pTXBuf[5] = HIBYTE(wNR);
	pTXBuf[6] = 0x01;				//1  类型标识TYP
	pTXBuf[8] = 0x14;				//20  传送原因COT(响应总召唤) 2 byte
	pTXBuf[9] = 0;
	pTXBuf[10] = LOBYTE(wUnitAddr);	//数据单元地址 2 byte
	pTXBuf[11] = HIBYTE(wUnitAddr);
	wPnt = YX_START_ADDR + m_wDataIndex;
	pTXBuf[12] = LOBYTE(wPnt);
	pTXBuf[13] = HIBYTE(wPnt);
	pTXBuf[14] = 0;
	byLen = 15;

	byCount = 0;
	WORD wSerialNo = 0 ;

	for(i=m_wDataIndex; i<m_wDISum ; i++)
	{
		GetRealVal(1, (WORD)i, &wVal);

		BYTE bySiq = 0 ;

		if( wVal )
			bySiq = 1 ;
		else
			bySiq = 0 ;

		wSerialNo = GetSerialNoFromTrans( YX_TRANSTOSERIALNO , i ) ;
		BOOL bDevState = FALSE ;
		bDevState = m_pMethod->GetDevCommState( wSerialNo ) ;
		if( bDevState == COM_DEV_ABNORMAL )
			bySiq |= 0xC0 ;

		pTXBuf[byLen] = bySiq ;
		byCount++;
		byLen += 1;
		if( byLen >= 15+127 ) break;
	}
	pTXBuf[7] = 0x80 | byCount;		//可变结构限定词VSQ
	pTXBuf[1] = byLen-2;			//长度L

	m_wDataIndex += byCount;
	if(m_wDataIndex>=m_wDISum)
	{
		SetTransIndex( 0 , 0 ) ;
		m_wCommand |= CMD_IGI_BIT;
	}
	return byLen;
}/*}}}*/

int CRtu104::LoadDigitalGroup(BYTE byGroup, WORD wUnitAddr, BYTE byReason)
{/*{{{*/
	WORD wNS, wNR, wIndex, wPnt, wVal;
	BYTE byLen, byCount;
	int  i;
	BYTE *pTXBuf = m_pTX_Buf;

	GetSendRecvNo( wNS , wNR ) ;
	pTXBuf[0] = 0x68;				//启动字符
	pTXBuf[2] = LOBYTE(wNS);
	pTXBuf[3] = HIBYTE(wNS);
	pTXBuf[4] = LOBYTE(wNR);
	pTXBuf[5] = HIBYTE(wNR);
	pTXBuf[6] = 0x01;				//类型标识TYP=1
	pTXBuf[8] = byReason;			//传送原因COT(响应分组召唤21-28) 2 byte
	pTXBuf[9] = 0;
	pTXBuf[10] = LOBYTE(wUnitAddr);	//数据单元地址 2 byte
	pTXBuf[11] = HIBYTE(wUnitAddr);

	wIndex = byGroup * 128;
	wPnt = YX_START_ADDR + wIndex;
	pTXBuf[12] = LOBYTE(wPnt);
	pTXBuf[13] = HIBYTE(wPnt);
	pTXBuf[14] = 0;
	byLen = 15;

	byCount = 0;
	for(i=wIndex; i<GetPntSum( YX_SUM ); i++)
	{
		GetRealVal(1, (WORD)i, &wVal);
		pTXBuf[byLen] = LOBYTE(wVal) & 0x81;
		byCount++;
		byLen += 1;
		if( byLen >= 15+127 ) break;
	}
	pTXBuf[7] = 0x80 | byCount;		//可变结构限定词VSQ
	pTXBuf[1] = byLen-2;			//长度L
	return byLen;
}/*}}}*/

int CRtu104::LoadPulseData(WORD wUnitAddr)
{/*{{{*/
	WORD wNS, wNR, wPnt;
	BYTE byLen, byCount;
	int  i;
	DWORD dwVal;
	BYTE *pTXBuf = m_pTX_Buf;

	GetSendRecvNo( wNS , wNR ) ;
	pTXBuf[0] = 0x68;				//启动字符
	pTXBuf[2] = LOBYTE(wNS);
	pTXBuf[3] = HIBYTE(wNS);
	pTXBuf[4] = LOBYTE(wNR);
	pTXBuf[5] = HIBYTE(wNR);
	pTXBuf[6] = 0x0f;				//15  类型标识TYP
	pTXBuf[8] = 0x25;				//37  传送原因COT(响应总召唤) 2 byte
	pTXBuf[9] = 0;
	pTXBuf[10] = LOBYTE(wUnitAddr);	//数据单元地址 2 byte
	pTXBuf[11] = HIBYTE(wUnitAddr);
	wPnt = PA_START_ADDR + m_wDataIndex;
	pTXBuf[12] = LOBYTE(wPnt);
	pTXBuf[13] = HIBYTE(wPnt);
	pTXBuf[14] = 0;
	byLen = 15;

	byCount = 0;
	for(i=m_wDataIndex; i<m_wPISum; i++)
	{
		dwVal = static_cast<DWORD>(m_dwPIBuf[i]);
		//BCR格式：24页 6.4.7
		pTXBuf[byLen]  = LOBYTE(LOWORD(dwVal));
		pTXBuf[byLen+1] = HIBYTE(LOWORD(dwVal));
		pTXBuf[byLen+2] = LOBYTE(HIWORD(dwVal));
		pTXBuf[byLen+3] = HIBYTE(HIWORD(dwVal));
		//处理品质描述词
		WORD wSerialNo = 0xFFFF;
		BYTE bySiq = 0 ;
		wSerialNo = GetSerialNoFromTrans( DD_TRANSTOSERIALNO , i ) ;
		BOOL bDevState = FALSE ;
		bDevState = m_pMethod->GetDevCommState( wSerialNo ) ;
		if( bDevState == COM_DEV_ABNORMAL )
			bySiq |= 0xC0 ;

		pTXBuf[byLen+4] = bySiq ;
		byCount++;
		byLen += 5;
		if( byLen >= 175 ) break;
	}
	pTXBuf[7] = 0x80 | byCount;		//可变结构限定词VSQ
	pTXBuf[1] = byLen-2;			//长度L

	m_wDataIndex += byCount;
	if(m_wDataIndex>=m_wPISum )
	{
		SetTransIndex( 0 , 0 ) ;
		SetCommand( CMD_YM_END );
	}

	return byLen;
}/*}}}*/

int CRtu104::LoadPulseGroup(BYTE byGroup, WORD wUnitAddr, BYTE byReason)
{/*{{{*/
	WORD wNS, wNR, wIndex, wPnt;
	BYTE byLen, byCount;
	int  i;
	QWORD dwVal;
	BYTE *pTXBuf = m_pTX_Buf;

	GetSendRecvNo( wNS , wNR ) ;
	pTXBuf[0] = 0x68;				//启动字符
	pTXBuf[2] = LOBYTE(wNS);
	pTXBuf[3] = HIBYTE(wNS);
	pTXBuf[4] = LOBYTE(wNR);
	pTXBuf[5] = HIBYTE(wNR);
	pTXBuf[6] = 0x0f;				//类型标识TYP=15
	pTXBuf[8] = byReason;			//传送原因COT(响应电量分组召唤38-41)
	pTXBuf[9] = 0;
	pTXBuf[10] = LOBYTE(wUnitAddr);	//数据单元地址 2 byte
	pTXBuf[11] = HIBYTE(wUnitAddr);

	wIndex = byGroup * 32;
	wPnt = PA_START_ADDR + wIndex;
	pTXBuf[12] = LOBYTE(wPnt);
	pTXBuf[13] = HIBYTE(wPnt);
	pTXBuf[14] = 0;
	byLen = 15;

	byCount = 0;
	for(i=wIndex; i<GetPntSum( YM_SUM ); i++)
	{
		//dwVal = m_dwPIBuf[i];
		dwVal = CalcPulseRipeVal(m_pPIMapTab[i].wStn,
				m_pPIMapTab[i].wPntNum,
				m_dwPIBuf[i]);
		DWORD val = static_cast<DWORD>(dwVal);
		//BCR格式：24页 6.4.7
		pTXBuf[byLen]  = LOBYTE(LOWORD(val));
		pTXBuf[byLen+1] = HIBYTE(LOWORD(val));
		pTXBuf[byLen+2] = LOBYTE(HIWORD(val));
		pTXBuf[byLen+3] = HIBYTE(HIWORD(val));
		pTXBuf[byLen+4] = 0;
		byCount++;
		byLen += 5;
		if( byLen >= 175 ) break;
	}
	pTXBuf[7] = 0x80 | byCount;		//可变结构限定词VSQ
	pTXBuf[1] = byLen-2;			//长度L
	return byLen;
}/*}}}*/

int CRtu104::LoadAllData(WORD wUnitAddr)
{/*{{{*/
	switch(m_byDataStyle & 0xf0)
	{
	case 0x10:
		//if(m_wRipeVal>=16)
		return LoadAnalogData13(wUnitAddr);
	case 0x11:												//永远娶不到!
		return LoadAnalogData21(wUnitAddr);
	case 0x20:
		return LoadDigitalData(wUnitAddr);
	case 0x30:
		return LoadPulseData(wUnitAddr);
	}
	return 0;
}/*}}}*/

int CRtu104::LoadRelayEchoFrame(WORD wUnitAddr, BYTE byType, BYTE byReason, WORD wIndexNum, BYTE byCmd)
{/*{{{*/
	WORD wPnt, wNS, wNR;
	BYTE byLen;
	BYTE *pTXBuf = m_pTX_Buf;

	GetSendRecvNo( wNS , wNR ) ;
	pTXBuf[0] = 0x68;				//启动字符
	pTXBuf[2] = LOBYTE(wNS);
	pTXBuf[3] = HIBYTE(wNS);
	pTXBuf[4] = LOBYTE(wNR);
	pTXBuf[5] = HIBYTE(wNR);
	byLen = 4;
	pTXBuf[6] = byType;				//   类型标识TYP=46/45
	pTXBuf[7] = 0x01;				//   可变结构限定词VSQ
	pTXBuf[8] = byReason;			//   传送原因COT
	pTXBuf[9] = 0;
	pTXBuf[10] = LOBYTE(wUnitAddr);	//数据单元地址 2 byte
	pTXBuf[11] = HIBYTE(wUnitAddr);
	wPnt = DO_START_ADDR + wIndexNum;
	pTXBuf[12] = LOBYTE(wPnt);		//信息体地址低 3 byte
	pTXBuf[13] = HIBYTE(wPnt);		//信息体地址高
	pTXBuf[14] = 0;
	pTXBuf[15] = byCmd;				//遥控命令DCO/SCO
	byLen += 10;

	pTXBuf[1] = byLen;				//长度L
	return byLen+2;
}/*}}}*/

int CRtu104::Resend_Proc( void )
{/*{{{*/
	return m_nTXCount;
}/*}}}*/

void CRtu104::RecvToutProc(void)
{/*{{{*/
}/*}}}*/

int CRtu104::AckMessageProc(BYTE* pDataBuf, int nLen)
{/*{{{*/
	return 0;
}/*}}}*/

int CRtu104::RelayCmdProc( BYTE byTypeID , BYTE byReason , WORD wInfoAddr , BYTE byVal )
{/*{{{*/
	WORD    wPnt = 0 , wNum = 0 ;
	WORD	wStn = 0;
	BYTE    byDCO = 0 , byStatus=0xff;
	int nSize = 0 ;
	wNum  = wInfoAddr - DO_START_ADDR;
	byDCO =  byVal ;			//buf[15];

	if( byTypeID != 45 && byTypeID != 46 )
	{
		return -1 ;
	}

	if( wInfoAddr < 0x6001 || wInfoAddr > 0x6200 )
	{
		return -1 ;
	}

	if( wNum >= GetPntSum( YK_SUM ) )
	{
		YK_ErrorProcess( byTypeID , wNum , wPnt , byDCO , byReason ) ;
		return -1;
	}

	wStn = m_pDOMapTab[wNum].wStn - 1 ;
	wPnt  = m_pDOMapTab[wNum].wPntNum - 1 ;

	BYTE bySDCO = GetS_DCO( byTypeID , byDCO ) ;
	switch( bySDCO )
	{
	case 0 :
		byStatus = 0x00;
		break;
	case 1:
		byStatus = 0x01;
		break;
	default:
		YK_ErrorProcess( byTypeID , wNum , wPnt , byDCO , byReason ) ;
		return -1 ;
	}

	if( byReason ==0x06 )
	{
		if(( byDCO & 0x80) != 0)			//预置
		{
			SetYKData( byTypeID , wNum , wPnt , byDCO , byReason ) ;
			nSize = RelaySelectProc( wStn , wPnt, byStatus );
			SetTTimer( IEC104_YKTIME , IEC104_YK_STARTTIME ) ;
		}
		else								//执行
		{
			BYTE byAction = 0 ;
			WORD wRelayNum = 0 ;
			BYTE byActNum = 0 ;
			BYTE byActMark = 0 ;
			WORD wSelectTime = 0 ;
			GetYkData( byAction , wRelayNum, byActNum , byActMark , wSelectTime ) ;

			if(
					( byAction == byTypeID ) &&
					(  wRelayNum  == wNum ) &&
					( ( byActMark & 0x7F ) == byDCO ) &&
					YK_IsCanSend()
			  )
			{
				SetYKData( byAction , wRelayNum , wPnt , byDCO , byReason ) ;
				RelayExecuteProc( wStn, wPnt, byStatus );
				SetTTimer( IEC104_YKTIME , IEC104_YK_STARTTIME ) ;
			}
			else
			{
				YK_ErrorProcess( byAction , wRelayNum , byActNum , byDCO , byReason ) ;
			}
		}
	}
	else if( byReason == 0x08 )				//取消!
	{
		if( YK_IsCanSend() )
		{
			SetYKData( byTypeID , wNum , wPnt , byDCO , byReason ) ;
			RelayCancelProc( wStn , wPnt, byStatus);
			SetTTimer( IEC104_YKTIME , IEC104_YK_STARTTIME ) ;
		}
		else
			YK_ErrorProcess( byTypeID , wNum , wPnt , byDCO , byReason ) ;
	}
	else
		YK_ErrorProcess( byTypeID , wNum , wPnt , byDCO , byReason ) ;

	return nSize;
}/*}}}*/

void CRtu104::YK_PreSet( BOOL bPreSetFlag )
{/*{{{*/
	m_YkFlag = bPreSetFlag ;
}/*}}}*/

BOOL CRtu104::YK_IsCanSend(  )
{/*{{{*/
	return m_YkFlag ;
}/*}}}*/

//返回：1是合 0为分 0xFF为非法命令
BYTE CRtu104::GetS_DCO( BYTE byTypeID , BYTE byVal )
{/*{{{*/
	if( ( byTypeID != 45 ) && ( byTypeID != 46 ) )
		return 0xFF ;

	if( byTypeID == 45 )
	{
		switch( byVal & 0x01 )
		{
		case 0:
			return FALSE ;
		case 1:
			return  TRUE ;
		}
	}
	else if( byTypeID == 46 )
	{
		switch( byVal & 0x03 )
		{
		case 1:
			return FALSE ;
		case 2:
			return  TRUE ;
		default:
			return 0xFF ;
		}
	}
	else
		return 0xFF ;

	return 0xFF ;
}/*}}}*/

void CRtu104::GetYkData( BYTE &byAction , WORD  &wRelayNum , BYTE & byActNum ,
		BYTE & byActMark , WORD & wReason)
{/*{{{*/
	byAction = m_byAction  ;
	wRelayNum  = m_wRelayNum;
	byActMark = m_byActMark ;
	wReason = m_wSelectTimer ;
}/*}}}*/
// byAction 类型标示
// wRelayNum 遥控编号
// byActNum  遥控点号
//	byActMark 遥控动作
// wSelectTime 遥控执行时间

void CRtu104::SetYKData( BYTE byAction , WORD  wRelayNum , BYTE byActNum ,
		BYTE byActMark , WORD wReason )
{/*{{{*/
	m_byAction = byAction ;
	m_wRelayNum = wRelayNum;
	m_wActNum = byActNum ;
	m_byActMark = byActMark ;
	m_wSelectTimer = wReason ;
}/*}}}*/

int CRtu104::RelaySelectProc(WORD wStn, WORD wCtrlNum, BYTE byStatus)
{/*{{{*/
	BYTE byBusNo;
	WORD wDevAddr;

	if(m_pMethod->GetBusLineAndAddr(wStn, byBusNo, wDevAddr))
		m_pMethod->SetYkSel(this, byBusNo, wDevAddr, wCtrlNum, byStatus);
	else
		printf("[Rtu104]:serialno err");
	return 0;
}/*}}}*/

int CRtu104::RelayExecuteProc(WORD wStn, WORD wCtrlNum, BYTE byStatus)
{/*{{{*/
	BYTE byBusNo;
	WORD wDevAddr;

	if(m_pMethod->GetBusLineAndAddr(wStn, byBusNo, wDevAddr))
		m_pMethod->SetYkExe(this, byBusNo, wDevAddr, wCtrlNum, byStatus);
	else
		printf("[Rtu104]:serialno err");
	return 0 ;
}/*}}}*/

int CRtu104::RelayCancelProc(WORD wStn, WORD wCtrlNum, BYTE byStatus)
{/*{{{*/
	BYTE byBusNo;
	WORD wDevAddr;

	if(m_pMethod->GetBusLineAndAddr(wStn, byBusNo, wDevAddr))
		m_pMethod->SetYkCancel(this, byBusNo, wDevAddr, wCtrlNum, byStatus);
	else
		printf("[Rtu104]:serialno err");
	return 0;
}/*}}}*/

int CRtu104::GroupReqProc(BYTE* pDataBuf, int nLen)
{/*{{{*/
	WORD wRtuAddr = MAKEWORD(pDataBuf[4], pDataBuf[5]);
	BYTE byQOI    = pDataBuf[9];

	if(byQOI>=29 && byQOI<=36)
		return LoadAnalogGroup(byQOI-29, wRtuAddr, byQOI);
	if(byQOI>=21 && byQOI<=28)
		return LoadDigitalGroup(byQOI-21, wRtuAddr, byQOI);

	return -1;
}/*}}}*/

int CRtu104::PulseReqProc(BYTE* pDataBuf, int nLen)
{/*{{{*/
	BYTE byQCC = pDataBuf[9] & 0x3f ;

	if( byQCC == 5 )
	{
		SetCommand( CMD_YM_CONFIRM ) ;
	}
	else
	{
		SetCommand( CMD_YM_ERROR ) ;
	}

	return -1;
}/*}}}*/

BOOL CRtu104::SysClockProc(BYTE* pDataBuf, int nLen)
{/*{{{*/
	if( pDataBuf == NULL || nLen < 16 )
		return FALSE ;

	REALTIME t;
	WORD     wMSecond;

	if(m_wRecvClock>0)
	{
		wMSecond = MAKEWORD(pDataBuf[9], pDataBuf[10]);
		t.wMilliSec = wMSecond % 1000;
		t.wSecond = wMSecond / 1000;
		t.wMinute = pDataBuf[11] & 0x3f;
		t.wHour   = pDataBuf[12] & 0x1f;
		t.wDay   = pDataBuf[13] & 0x1f;
		t.wMonth = pDataBuf[14] & 0x0f;
		if(pDataBuf[15]>30)
			t.wYear  = pDataBuf[15] + 1970;
		else
			t.wYear  = pDataBuf[15] + 2000;

		if( t.wSecond < 60 &&
				t.wMinute < 60 &&
				t.wHour < 24 &&
				t.wDay <= 31 &&
				t.wMonth <= 12 &&
				t.wYear < 2030  )

			printf( "IEC104 time %d-%d-%d %d-%d-%d" , t.wYear , t.wMonth,t.wDay,t.wHour,t.wMinute,t.wSecond );
			SetCurrentTime( &t );
			system("hwclock -w -u"); //将linux系统时间同步到RTC 否则系统重启后 时间会恢复到以前
	}

	return TRUE ;
}/*}}}*/

int	CRtu104::DealBusMsgInfo(PBUSMSG pBusMsg)
{/*{{{*/

	switch ( pBusMsg->byMsgType )
	{
	case YK_PROTO:
	{
		YK_DATA *pData = (YK_DATA *)(pBusMsg->pData);
		WORD wSerialNo;
		switch (pBusMsg->dwDataType)
		{
		case YK_SEL_RTN:
		case YK_EXCT_RTN:
		case YK_CANCEL_RTN:
		{
			if (pBusMsg->DataNum != 1
				|| pBusMsg->DataLen != sizeof(YK_DATA))
			{
				printf("IEC104 Yk DataNum err\n");
				return -1;
			}

			int temp = m_pMethod->GetSerialNo(pBusMsg->SrcInfo.byBusNo, pBusMsg->SrcInfo.wDevNo);
			if (temp == -1)
				return -1;
			else
				wSerialNo = (WORD)temp;

			if (pData->byVal == YK_ERROR)
			{
				//回复IEC104否定认可
				//回复IEC104否定认可
				BYTE byAction = 0;
				WORD  wRelayNum = 0;
				BYTE byActNum = 0;
				BYTE byActMark = 0;
				WORD wReason = 0;
				GetYkData(byAction, wRelayNum, byActNum, byActMark, wReason);
				YK_ErrorProcess(byAction, wRelayNum, byActNum, byActMark, wReason);
			}
			else
			{
				RelayProc((BYTE)pBusMsg->dwDataType, wSerialNo, pData->wPnt, (BYTE)pData->byVal);
			}

			break;
		}

		default:
		{
			printf("IEC104 can't find the YK_DATATYPE %d\n", (int)pBusMsg->dwDataType);
			break;
		}
		}				/* -----  end switch  ----- */
	}
		break;
	case DZ_PROTO:
		ProcessDz(pBusMsg);
		break;
	default:
		break;
	}				/* -----  end switch  ----- */
	return 1;
}/*}}}*/

int CRtu104::ProcessDz(PBUSMSG pBusMsg)
{
	DZ_DATA * pDzData = (DZ_DATA *)(pBusMsg->pData);
	const BYTE READ_OPT = 0x01;
	const BYTE WRITE_OPT = 0x10;

	if (m_byDzOpt == WRITE_OPT)
		SetCommand(CMD_DZ_WRITE_CONFIRM);
	else if (m_byDzOpt == READ_OPT)
	{
		int dataNum = pBusMsg->DataNum - 1; //第一个定值为定值组信息 不处理
		DZ_DATA * pDz = pDzData + 1;
		for (int i = 0; i < dataNum; i++)
		{
            int ir = pDz->byType ;
            float t = 0 ;
            memcpy(&t , pDz->byVal , sizeof( float ) ) ;
			memcpy(m_ReadzData[i].byVal, pDz->byVal, sizeof(float));
			m_ReadzData[i].byType = pDz->byType;
			m_ReadzData[i].wPnt = pDz->wPnt;
			pDz = pDz + 1;
		}

		SetCommand(CMD_DZ_READ_CONFIRM);
	}
}

/******************************************************************************
 * 任务处理函数
 */
void CRtu104::TaskProcHandle(void)
{}
/******************************************************************************/

BOOL CRtu104::Init( BYTE byLineNo )
{/*{{{*/
	m_byLineNo = byLineNo ;
	m_byProID    = 4;
	m_byEnable   = 1;
	m_wObjNum  = 1;
	sprintf( m_szObjName, "%s", m_sDevName );
	sprintf( m_ComCtrl1, "%s", m_sMasterAddr );
	m_wRtuAddr   = m_wDevAddr ;
	char szFileName[256] = "";

	sprintf( szFileName, "%s%s", IEC104PREFIXFILENAME, m_sTemplatePath );
	//读取需要转发的数据到该模块
	ReadMapConfig( szFileName );

	//初始化该模块
	InitRtuBase() ;

	//初始化上传装置状态时间
	//time( &m_DevStateTime ) ;
	m_DevStateTime = 0;

	return TRUE ;
}/*}}}*/

int  CRtu104::StartedProcess( WORD wAddr )
{/*{{{*/
	int len = 0 ;
	if( GetCommand() == CMD_TIME_CONFIRM )
	{
		len = SysClockConfirm( wAddr ) ;
		SetCommand( CMD_NULL ) ;
	}
	else if( GetCommand() == CMD_TOTAL_CONFIRM )
	{
		len = AllDataEcho(m_wRtuAddr, 0x07);
		SetCommand( CMD_NULL ) ;
	}

	else if( m_byDataStyle != 0 )					//上送总召唤数据
		len = LoadAllData(wAddr);

	else if( (m_wCommand & CMD_IGI_BIT) != 0 )		//总召唤结束
	{
		m_wCommand &= ~CMD_IGI_BIT;
		len = AllDataEcho(wAddr, 10);
	}
	else if( GetCommand() == CMD_YM_CONFIRM )
	{
		SetTransIndex( 0 , 0x30 ) ;
		SetCommand( CMD_NULL ) ;
		len = ReqPulseEcho( wAddr , 7, 0x05 );
	}
	else if( GetCommand( ) == CMD_YM_ERROR)
	{
		len = ReqPulseEcho( wAddr , 47, 0x05 );
		SetCommand( CMD_NULL ) ;
	}

	else if( GetCommand( ) == CMD_YM_END )
	{
		len = ReqPulseEcho( wAddr , 10 , 0x05 );
		SetCommand( CMD_NULL ) ;
	}
	else
		len = ChangeDataProcess( wAddr ) ;

	return len ;
}/*}}}*/

int CRtu104::PreProcess( WORD wAddr )
{/*{{{*/
	int len = 0 ;

	if((m_wCommand & CMD_TEST_BIT) != 0)		//测试帧确认
	{
		len = TestFrameAck();
		m_wCommand &= ~CMD_TEST_BIT;
	}
	else if( m_wCommand & CMD_TEST_SEND ) 		//启动测试帧
	{
		len = TestSend( ) ;
		m_wCommand &= ~CMD_TEST_SEND ;

		//关闭T3
		SetTTimer( IEC104_T3 , IEC104_END_T3 ) ;
		SetTTimer( IEC104_T1 , IEC104_START_T1 ) ;
	}
	else if((m_wCommand & CMD_START_BIT) != 0)	//启动数据传输			//m_wCommand:由ProcessProtocolBuf:buf[2]置位!
	{
		len = StartDataAck();
		m_wCommand &= ~CMD_START_BIT;
		m_bStartBit = TRUE;
	}
	else if((m_wCommand & CMD_STOP_BIT) != 0)	//停止数据传输
	{
		len = StopDataAck();
		m_wCommand &= ~CMD_STOP_BIT;
		m_bStartBit = FALSE;
	}
	else if((m_wCommand & CMD_UFACK_BIT) != 0)	//发送接收确认帧
	{
		len = AckFrame();
		m_wCommand &= ~CMD_UFACK_BIT;

		//重置T2
		SetTTimer( IEC104_T2 , IEC104_START_T2 ) ;
	}

	else if( (m_wCommand & CMD_COA_BIT) != 0 )	//遥控命令响应
	{
		YK_Rtn( len ) ;
		m_wCommand &= ~CMD_COA_BIT;
	}
	else if( GetCommand() == CMD_YK_ERROR )
	{
		BYTE byAction = 0 ;
		WORD  wRelayNum = 0 ;
		BYTE byActNum = 0 ;
		BYTE byActMark = 0 ;
		WORD wReason = 0 ;
		GetYkData( byAction , wRelayNum , byActNum , byActMark , wReason ) ;

		len = LoadRelayEchoFrame( m_wRtuAddr, byAction, wReason, wRelayNum ,  byActMark );
		SetCommand( CMD_NULL ) ;
	}
	else if (GetCommand() == CMD_DZ_WRITE_CONFIRM)
	{
		len = LoadDzEchoFrame( );
		SetCommand(CMD_NULL ) ;
	}
	else if (GetCommand() == CMD_DZ_READ_CONFIRM)
	{
		len = LoadReadDzEchoFrame();
		SetCommand(CMD_NULL);
	}

	return len ;
}/*}}}*/

int CRtu104::LoadReadDzEchoFrame()
{
	WORD wPnt, wNS, wNR;
	BYTE *pTXBuf = m_pTX_Buf;

	GetSendRecvNo(wNS, wNR);
	pTXBuf[0] = 0x68;				//启动字符
	pTXBuf[2] = LOBYTE(wNS);
	pTXBuf[3] = HIBYTE(wNS);
	pTXBuf[4] = LOBYTE(wNR);
	pTXBuf[5] = HIBYTE(wNR);
	pTXBuf[6] = 146;				//类型标识
	pTXBuf[7] = m_byDzNum;				//   可变结构限定词VSQ
	pTXBuf[8] = 0x0F;			//   传送原因COT
	pTXBuf[9] = 0;
	pTXBuf[10] = LOBYTE(m_wRtuAddr);	//数据单元地址 2 byte
	pTXBuf[11] = HIBYTE(m_wRtuAddr);
	int infoAddr = 0;

	int index = 12;
	int dz_num = m_byDzNum;
	BYTE byDzStartNo = m_byDzStartNo;
	for (int i = 0; i < dz_num; i++)
	{
		pTXBuf[index++] = LOBYTE(infoAddr);		//信息体地址低 3 byte
		pTXBuf[index++] = HIBYTE(infoAddr);		//信息体地址高
		pTXBuf[index++] = 0;

		pTXBuf[index++] = byDzStartNo++;

		float t = 0;
		memcpy(&t, m_ReadzData[i].byVal, sizeof(float));
		memcpy(pTXBuf + index, m_ReadzData[i].byVal, sizeof(float));
		index += 4;
	}

	pTXBuf[1] = index - 2;				//长度L
	return index;
}

int CRtu104::LoadDzEchoFrame()
{
	WORD wPnt, wNS, wNR;
	BYTE *pTXBuf = m_pTX_Buf;

	GetSendRecvNo(wNS, wNR);
	pTXBuf[0] = 0x68;				//启动字符
	pTXBuf[2] = LOBYTE(wNS);
	pTXBuf[3] = HIBYTE(wNS);
	pTXBuf[4] = LOBYTE(wNR);
	pTXBuf[5] = HIBYTE(wNR);
	pTXBuf[6] = 146;				//类型标识
	pTXBuf[7] = m_byDzNum;				//   可变结构限定词VSQ
	pTXBuf[8] = 0x0F;			//   传送原因COT
	pTXBuf[9] = 0;
	pTXBuf[10] = LOBYTE(m_wRtuAddr);	//数据单元地址 2 byte
	pTXBuf[11] = HIBYTE(m_wRtuAddr);
	int infoAddr = 0;

	int index = 12;
	int dz_num = m_byDzNum;
	BYTE byDzStartNo = m_byDzStartNo ;
	for (int i = 0; i < dz_num; i++)
	{
		pTXBuf[index++] = LOBYTE(infoAddr);		//信息体地址低 3 byte
		pTXBuf[index++] = HIBYTE(infoAddr);		//信息体地址高
		pTXBuf[index++] = 0;

		pTXBuf[index++] = byDzStartNo++;

		for( int m = 0 ; m < 4 ; m++ )
			pTXBuf[ index++ ] = 0 ;				//数据全部为0
	}

	pTXBuf[1] = index - 2 ;				//长度L
	return index ;
}

void CRtu104::YK_Rtn( int &len )
{/*{{{*/
	BYTE byAction = 0 ;
	WORD  wRelayNum = 0 ;
	BYTE byActNum = 0 ;
	BYTE byActMark = 0 ;
	WORD wSelectTime = 0 ;
	GetYkData( byAction , wRelayNum , byActNum , byActMark , wSelectTime ) ;

	if(m_wYkFlag  ==  YK_SEL_RTN )
	{
		len = LoadRelayEchoFrame(m_wRtuAddr, byAction, 0x07, wRelayNum, 0x80 | byActMark);
		YK_RtnConfirm( TRUE ) ;
	}
	else if(m_wYkFlag  == YK_EXCT_RTN )
	{
		len = LoadRelayEchoFrame(m_wRtuAddr, byAction, 0x07, wRelayNum, byActMark);
		YK_RtnConfirm( FALSE ) ;
	}
	else if( m_wYkFlag == YK_CANCEL_RTN )//遥控没有预置不能取消
	{
		len = LoadRelayEchoFrame(m_wRtuAddr, byAction, 0x09, wRelayNum, 0x80 | m_byActMark);
		YK_RtnConfirm( FALSE ) ;
	}

}/*}}}*/

void CRtu104::YK_RtnConfirm( BOOL bFlag )
{/*{{{*/
	YK_PreSet( bFlag ) ;
	SetTTimer( IEC104_YKTIME , IEC104_YK_ENDTIME ) ;
}/*}}}*/

int CRtu104::ChangeDataProcess( WORD wAddr )
{/*{{{*/
	int len = 0 ;

	//利用空闲状态每10秒转发一次装置状态
	time_t timeTemp ;
	time( &timeTemp ) ;

	if( m_dwDIEQueue.size() > 0 )              //传输遥信变位
		len = LoadDIEFrame( wAddr );

	else if( m_iSOE_rd_p != m_iSOE_wr_p )	   //传输SOE信息
		len = LoadSOEFrame_30(wAddr);

	else if( difftime( timeTemp , m_DevStateTime ) > 10  ) //单位秒	noted by cyz!
	// else if(!QueryAllDevStatus && (deque_stat.size() != 0) || (difftime( timeTemp , m_DevStateTime ) > 60 * 15 )) //单位秒	by cyz!
	{
		QueryAllDevStatus = TRUE;
		len = ComState_Message( wAddr ) ;
		time( &m_DevStateTime ) ;
	}
	else if( m_dwAIEQueue.size() > 0 )
	//else if( counter > 0 )			//+ by cyz!	for 兰大二院.
	{
		//            if(m_wRipeVal>=16)
		//                len = LoadAIEFrame13(wAddr);
		//            else
		//					len = LoadAIEFrame21(wAddr);
		len = LoadAIEFrame13(wAddr);
	}

	return len ;
}/*}}}*/

BOOL CRtu104::IsCanSend( )
{/*{{{*/
	if( m_wNumSend - m_wNumAck >= IEC104_Q )
	{
		//如果大于IEC104_Q 仍然不给序号确认,则停止发送新数据
		return FALSE ;
	}
	else if( m_wNumSend < m_wNumAck )
	{
		return FALSE;
	}

	return TRUE ;
}/*}}}*/

BOOL CRtu104::LoadRtuMessage( BYTE * pBuf , int &len )
{/*{{{*/
	if( pBuf == NULL  )
		return FALSE ;

	WORD  wAddr;
	wAddr = m_wRtuAddr;
	m_pTX_Buf = pBuf ;

	len = PreProcess( wAddr );
	if( len > 0 )
		;
	else
	{
		if( m_bStartBit )					//m_bStartBit:只在PreProcess中被置为真
		{
			//if( !IsCanSend( ))
			//	return FALSE ;

			len = StartedProcess( wAddr ) ;
		}
	}

	if( len <= 0 )
		return FALSE;
	else
	{/*{{{*/
		//如果是I帧发送序号加1
		if( (m_pTX_Buf[2] & 0x01) == 0 )
		{
			m_wNumSend = (m_wNumSend+1) % MAXMUM_NUM;
			//重置T2, T3
			SetTTimer( IEC104_T2 , IEC104_START_T2 ) ;
			SetTTimer( IEC104_T3 , IEC104_START_T3 ) ;
		}
		m_wTimerCount= 0;
	}/*}}}*/

	return TRUE ;
}/*}}}*/

BOOL CRtu104::GetProtocolBuf( BYTE * buf , int &len , PBUSMSG pBusMsg  )
{/*{{{*/
	if( buf == NULL )
		return FALSE ;

	if( pBusMsg )
	{
		DealBusMsgInfo(pBusMsg);
	}

	//如果端口不通，重置IEC104所有状态
	if( !m_pMethod->IsPortValid() )
	{
		ReSetState() ;
	}

	LoadRtuMessage(  buf , len )  ;
	//printf("\n---------------------------------------------------\n");
	//for(int i = 0; i < len; i++)
	//printf("%02x ", buf[i]);
	//printf("\n---------------------------------------------------\n\n");
	return TRUE;
}/*}}}*/

BOOL CRtu104::ProcessProtocolBuf( BYTE * pbuf , int len )
{/*{{{*/
	//printf("\n===================================================\n");
	//for(int i = 0; i < len; i++)
	//printf("%02x ", pbuf[i]);
	//printf("\n===================================================\n\n");
	if( len == 0 || pbuf == NULL )
		return FALSE ;
	/*lel*/
#if 1
	BYTE bLen = len;
	BYTE bNLen = pbuf[1] + 2;
	if(bLen > bNLen)
	{
		RtuCommandProc(pbuf, bNLen);
		BYTE bLen1 = len - bNLen;
		BYTE bNLen1 = pbuf[bNLen + 1] + 2;
		if(bLen1 > bNLen1)
		{
			RtuCommandProc(pbuf + bNLen, bNLen1);
		//	BYTE bLen2 = len - bNLen - bNLen1;
			BYTE bNLen2 = pbuf[bNLen + bNLen1 + 1] + 2;
			RtuCommandProc(pbuf + bNLen + bNLen1, bNLen2);
		}
		else if(bLen1 == bNLen1)
			RtuCommandProc( pbuf + bNLen, bLen1);
	}
	else if(bLen == bNLen)
		RtuCommandProc( pbuf , len );
#else
	RtuCommandProc( pbuf , len );
#endif
	/*end*/

	return  TRUE ;
}/*}}}*/

BOOL CRtu104::GetStartDt( )
{/*{{{*/
	return m_bStartBit ;
}/*}}}*/

WORD CRtu104::GetSeqNo( BYTE * pBuf )
{/*{{{*/
	WORD wSeqNo;

	if(pBuf==NULL)
		return 0xFFFF;	/*无效*/

	wSeqNo=MAKEWORD(*pBuf ,*( pBuf+1 ) );
	wSeqNo>>=1;

	return wSeqNo;
}/*}}}*/

BOOL CRtu104::ProcRecvNo( WORD wRecNo )
{/*{{{*/
//	if( wRecNo != m_wNumRecv )
//		return FALSE;

	m_wNumRecv++;
	if( m_wNumRecv >= MAXMUM_NUM )
		m_wNumRecv = 0 ;

	if( m_wNumRecBK > m_wNumRecv
			&& m_wNumRecv+MAXMUM_NUM - m_wNumRecBK >= IEC104_W )
	{
		//发送S帧序号确认
		m_wCommand |= CMD_UFACK_BIT;
	}
	//接受到I帧大于8个时,以S帧确认
	else if( m_wNumRecBK < m_wNumRecv
			&& wRecNo- m_wNumRecBK >= IEC104_W )
	{
		//发送S帧序号确认
		m_wCommand |= CMD_UFACK_BIT;
	}

	return TRUE ;
}/*}}}*/

BOOL CRtu104::ProcSendNo( WORD wAckNo )
{/*{{{*/
	/*确认序号和上次的一样，则不需要处理*/
	if( m_wNumAck == wAckNo)
		return TRUE;

// 	if( m_wNumSend > m_wNumAck )
// 	{
// 		if( wAckNo < m_wNumAck || wAckNo> m_wNumSend )
// 			return FALSE;
// 	}
// 	else if( m_wNumSend < m_wNumAck )
// 	{
// 		if( wAckNo > m_wNumSend && wAckNo<m_wNumAck )
// 			return FALSE;
// 	}
// 	else
// 	{
// 		if(wAckNo!=m_wNumSend )
// 			return FALSE;
// 	}

	m_wNumAck = wAckNo ;

	/*判断发送序号是否全部确认，如果没有，则启动T4计时*/
	if( m_wNumAck!= m_wNumSend )
	{
		SetTTimer( IEC104_T4 , IEC104_START_T4 );
	}
	else
	{
		SetTTimer( IEC104_T4 , IEC104_END_T4 );
	}

	return TRUE ;
}/*}}}*/

//////////////////////////////   主站报文处理   ///////////////////////////////////
void CRtu104::RtuCommandProc(BYTE* pRecvBuf, int nLen)
{/*{{{*/
	BYTE    byCmd;
	WORD    wNS, wNR;
	WORD wIndex = 0 ;
	WORD len = 0 ;

	//将接收数据放入本地缓存区
	m_pRX_Buf = pRecvBuf ;
	m_nRXCount = nLen;

	if( m_nRXCount < 1 )
		return ;

	if( m_pRX_Buf[ wIndex++ ] != 0x68 )
		return ;

	len = m_pRX_Buf[ wIndex] + 2;

	if( m_nRXCount != len )
		return ;

	byCmd = pRecvBuf[2];
	switch(byCmd & 0x03)								//帧类别!刚建立连接的时候先发送U帧
	{
	case 0x00:  //编号的信息传输(I格式)
	case 0x02:
		{
			if( GetStartDt() == FALSE )
				return  ;

			//获取对端发送序列号
			wNR = GetSeqNo( pRecvBuf + 2 ) ;

			//处理序列号
			if( !ProcRecvNo( wNR ) )
			{
				ReSetState() ;
				if( m_pMethod )
					m_pMethod->CloseSocket( m_byLineNo ) ;
				return  ;
			}

			//获取对端接受序列号
			wNS = GetSeqNo( pRecvBuf + 4 ) ;

			//处理序号
			if( !ProcSendNo( wNS ) )
			{
				ReSetState() ;
				if( m_pMethod )
					m_pMethod->CloseSocket( m_byLineNo ) ;
				return ;
			}

			if(nLen > 6)
			{
				InfoMessageProc(&pRecvBuf[6], nLen-6);
				SetTTimer( IEC104_T2 , IEC104_START_T2 ) ;
				SetTTimer( IEC104_T3 , IEC104_START_T3 ) ;
				QueryAllDevStatus = TRUE;
			}
		}
		break;
	case 0x01:	//编号的监视功能(S格式)
		{
			wNR = MAKEWORD(pRecvBuf[4], pRecvBuf[5])>>1;

			//处理序号
			if( !ProcSendNo( wNR ) )
			{
				ReSetState() ;
				if( m_pMethod )
					m_pMethod->CloseSocket( m_byLineNo ) ;
				return ;
			}

			SetTTimer( IEC104_T3 , IEC104_START_T3 ) ;
		}
		break;
	case 0x03:	//未编号的控制功能(U格式)
		{
			if((byCmd & 0x04) != 0)
			{
				ReSetState( ) ;

				m_wCommand |= CMD_START_BIT;
				//启动 T2 , T3 时间
				SetTTimer( IEC104_T2 , IEC104_START_T2 ) ;
				SetTTimer( IEC104_T3 , IEC104_START_T3 ) ;
			}
			else if((byCmd & 0x10) != 0)
				m_wCommand |= CMD_STOP_BIT;
			else if((byCmd & 0x40) != 0)
			{
				m_wCommand |= CMD_TEST_BIT;
				SetTTimer( IEC104_T3 , IEC104_START_T3 ) ;
			}
			else if( ( byCmd & 0x80 ) !=0 )
			{
				//关闭时间t1
				SetTTimer( IEC104_T1 , IEC104_END_T1 ) ;
				SetTTimer( IEC104_T3 , IEC104_START_T3 ) ;
			}
		}
		break;
	default:
		return ;
	}

	return;
}/*}}}*/

BOOL CRtu104::ReSetState( )
{/*{{{*/
	//复位接受计数
	m_wNumRecv  = 0;
	m_wNumRecBK = 0;

	//复位发送计数
	m_wNumSend  = 0;
	m_wNumAck  = 0;

	//重置超时时间
	m_byTimeFlag = 0 ;
	//重置时间
	SetTTimer( IEC104_T1 ,  IEC104_END_T1 ) ;
	SetTTimer( IEC104_T2 ,  IEC104_END_T2 ) ;
	SetTTimer( IEC104_T3 ,  IEC104_END_T3 ) ;
	SetTTimer( IEC104_T4 ,  IEC104_END_T4 ) ;
	SetTTimer( IEC104_YK_STARTTIME ,  IEC104_YK_ENDTIME ) ;

	//StartDT退出
	m_bStartBit = FALSE ;

	//清除发送标志
	m_wCommand = 0 ;

	//重置遥控标志
	YK_PreSet( FALSE ) ;
	m_YKTime = 0 ;

	//重置--传输装置状态的时间
	//time( &m_DevStateTime ) ;
	m_DevStateTime = 0;

	return TRUE ;
}/*}}}*/

//启动计时
BOOL CRtu104::SetTTimer( BYTE byTime , BYTE byState )
{/*{{{*/
	//printf("********	line:%d byTime:%d byState:%d	********\n", __LINE__, byTime, byState);
	switch( byTime )
	{
	case IEC104_T1:
		{
			SetIEC104Time( &m_t1 , byState , IEC104_START_T1 , IEC104_END_T1 ) ;
		}
		break;
	case IEC104_T2:
		{
			SetIEC104Time( &m_t2 , byState , IEC104_START_T2 , IEC104_END_T2 ) ;
		}
		break;
	case IEC104_T3 :
		{
			SetIEC104Time( &m_t3 , byState , IEC104_START_T3 , IEC104_END_T3 ) ;
		}
		break;
	case IEC104_T4 :
		{
			SetIEC104Time( &m_t4 , byState , IEC104_START_T4 , IEC104_END_T4 ) ;
		}
		break;
	case IEC104_YKTIME:
		{
			SetIEC104Time( &m_YKTime , byState , IEC104_YK_STARTTIME , IEC104_YK_ENDTIME ) ;
		}
		break;
	default:
		return FALSE ;
	}

	return TRUE ;
}/*}}}*/

void CRtu104::SetIEC104Time( time_t *tTime , BYTE byState , BYTE byBeginT , BYTE byEndT )
{/*{{{*/
	if( byState == byBeginT )
	{
		m_byTimeFlag |= byBeginT ;
		time( tTime ) ;
	}
	else if( byState == byEndT )
	{
		m_byTimeFlag &= byEndT ;
	}
}/*}}}*/

BYTE CRtu104::GetTimeFlag( )
{/*{{{*/
	return m_byTimeFlag  ;
}/*}}}*/

int CRtu104::InfoMessageProc(BYTE* pDataBuf, int nLen)
{/*{{{*/
	int  nSize = 0;
	BYTE byIndex = 0 ;
	BYTE byTypeID = pDataBuf[ byIndex++ ];				//byTypeID=buf[6]	byTypeID:根据后台配置文件可以区别单点双点!单点=45 双点=46

	//可变结构限定词
	if( pDataBuf[ byIndex++ ] != 0x01 )
		return 0 ;
	//传输原因
	BYTE byReason = pDataBuf[ byIndex++] & 0x3f;		//byReason = buf[8] & 0x3f;
	if( ( ( byReason >> 6 ) & 0x01 ) != 0 )
		return 0 ;

	byIndex++ ;

	//公共地址
	WORD wRtuAddr = MAKEWORD(pDataBuf[ byIndex ], pDataBuf[ byIndex + 1 ]);
	if( wRtuAddr != m_wRtuAddr )
		return 0 ;

	byIndex += 2 ;

	//信息体地址
	WORD wInfoAddr = MAKEWORD(pDataBuf[ byIndex ], pDataBuf[ byIndex + 1 ] ) ;
	byIndex += 3  ;

	switch( byTypeID )
	{
	case 45: //单点遥控命令
	case 46: //双点遥控命令
		{
			BYTE SCO = pDataBuf[ byIndex ] ;				//SCO = buf[15];
			if(byReason==0x06 || byReason==0x08)
				nSize = RelayCmdProc ( byTypeID , byReason , wInfoAddr , SCO );
		}
		break;
	case 47: //升降命令
		break;
	case 100: //总召唤
		if(byReason==0x06) 	//总召唤激活
		{
			SetTransIndex( 0 , 0x10 ) ;
			m_dwAIEQueue.clear();
			SetCommand( CMD_TOTAL_CONFIRM );
			//从内存数据库中--获取转发表默认数据
			m_pMethod->ReadAllYcData(&m_wAIBuf[0]);
			m_pMethod->ReadAllYmData(&m_dwPIBuf[0]);
			m_pMethod->ReadAllYxData(&m_byDIbuf[0]);
		}
		/*
		   else if(byReason==0x08)	//总召唤停止
		   {
		   m_bAllData = FALSE;
		   m_byDataStyle = 0;
		   m_wDataIndex  = 0;
		   nSize = AllDataEcho(m_wRtuAddr, 0x09);
		   }
		   */
		/*        else if(byReason==0x05)	//分组召唤(见Page67)
				  {
				  nSize = GroupReqProc(pDataBuf, nLen);
				  }
				  */
		break;
	case 101: //召唤电能脉冲计数
		{
			if( byReason == 0x06 )
			{
				m_pMethod->ReadAllYmData(&m_dwPIBuf[0]);
				nSize = PulseReqProc(pDataBuf, nLen);
			}
		}
		break;
	case 103: //时钟同步命令
		SetCommand( CMD_TIME_CONFIRM ) ;
		nSize = SysClockProc(pDataBuf, nLen);
		break;
	case 145:
	{
		DzProcess(pDataBuf + byIndex  );
	}
	break;
	case 105: //复位进程命令
		break;
	case 107: //带时标CP56Time2a的测试命令
		break;
	default:
		return -1;
	}

	return nSize;
}/*}}}*/

void CRtu104::DzProcess(BYTE * pBuf)
{
	BYTE index = 0;
	BYTE byChannle = pBuf[index++];
	BYTE byBusNo = pBuf[index++];
	BYTE byAddr = pBuf[index++];
	m_byDzOpt = pBuf[index++];
	BYTE byGroupNo = pBuf[index++];
	m_byDzNum = pBuf[index++];

	const BYTE READ_OPT = 0x01;
	const BYTE WRITE_OPT = 0x10;
	if (m_byDzOpt == READ_OPT)
	{
		DzRead(byBusNo, byAddr, m_byDzNum, pBuf + index);
	}
	else if(m_byDzOpt == WRITE_OPT )
	{
		DzWrite(byBusNo, byAddr, m_byDzNum, pBuf + index );
	}
}

void CRtu104::DzRead(BYTE byBusNo, BYTE byAddr, BYTE byNum, BYTE * pBuf)
{
	WORD wDevAddr = byAddr;
	if (byNum > 100)
		byNum = 100;
	WORD windex = 0;
	m_byDzStartNo = pBuf[windex];
	DZ_DATA dzData[100];
	for (BYTE i = 0; i < byNum; i++)
	{
		dzData[i].wPnt = pBuf[windex++];
		memset(dzData[i].byVal, 0, sizeof(float));
		windex += 4;
	}

	m_pMethod->SetDzCall(this, byBusNo, wDevAddr, 0, (DZ_DATA *)&dzData, byNum);
}

void CRtu104::DzWrite(BYTE byBusNo, BYTE byAddr, BYTE byNum, BYTE * pBuf)
{
	WORD wDevAddr=byAddr;
	if (byNum > 100)
		byNum = 100;

	WORD windex = 0;
	DZ_DATA dzData[100] ;
	m_byDzStartNo = pBuf[windex];

	for (BYTE i = 0; i < byNum; i++ )
	{
		dzData[i].wPnt = pBuf[windex++];
		memcpy(dzData[i].byVal, pBuf + windex, sizeof(float));
		const BYTE FLOAT_TYPE = 3;
		dzData[i].byType = FLOAT_TYPE;
		windex += 4;
	}

	m_pMethod->SetDzWriteExct(this, byBusNo, wDevAddr, 0, (DZ_DATA *)&dzData, byNum);
}

void CRtu104::SetTransIndex( WORD Index , WORD DataStyle )
{/*{{{*/
	m_wDataIndex = Index ;
	m_byDataStyle = DataStyle ;
}/*}}}*/

BOOL CRtu104::GetSendRecvNo( WORD &wSend , WORD &wRecv )
{/*{{{*/
	wSend = m_wNumSend<<1;
	wRecv = m_wNumRecv<<1;

	m_wNumRecBK = m_wNumRecv ;

	return TRUE ;
}/*}}}*/

int CRtu104::SysClockConfirm( WORD wAddr )
{/*{{{*/
	WORD     wNS, wNR;
	WORD     wMSecond;
	REALTIME t;
	GetSendRecvNo( wNS , wNR ) ;
	m_pTX_Buf[0] = 0x68;				//启动字符
	m_pTX_Buf[1] = 0 ;
	m_pTX_Buf[2] = LOBYTE(wNS);
	m_pTX_Buf[3] = HIBYTE(wNS);
	m_pTX_Buf[4] = LOBYTE(wNR);
	m_pTX_Buf[5] = HIBYTE(wNR);
	//链路用户数据
	m_pTX_Buf[6] = 0x67;			//类型标识(103)
	m_pTX_Buf[7] = 0x01;			//可变结构限定词
	m_pTX_Buf[8] = 0x07;			//传送原因
	m_pTX_Buf[9] = 0;
	m_pTX_Buf[10] = LOBYTE( wAddr );	//数据单元地址 2 byte
	m_pTX_Buf[11] = HIBYTE( wAddr );
	m_pTX_Buf[12] = 0;				//信息体地址 3 byte
	m_pTX_Buf[13] = 0;
	m_pTX_Buf[14] = 0;

	//信息元素(见71页)
	GetCurrentTime( &t );
	wMSecond = t.wSecond*1000 + t.wMilliSec;
	m_pTX_Buf[15] = LOBYTE(wMSecond);			//毫秒L
	m_pTX_Buf[16] = HIBYTE(wMSecond);			//毫秒H
	m_pTX_Buf[17] = (BYTE)t.wMinute;			//分
	m_pTX_Buf[18] = (BYTE)t.wHour;				//时
	m_pTX_Buf[19] = LOBYTE((t.wDayOfWeek<<5)+t.wDay);	//日b7--b5: 星期
	m_pTX_Buf[20] = (BYTE)t.wMonth;				//月
	m_pTX_Buf[21] = t.wYear	- 2000	;		//年

	m_pTX_Buf[1] = 22 - 2 ;					//长度L
	return 22 ;
}/*}}}*/

BOOL CRtu104::SetCommand( WORD wCommand )
{/*{{{*/
	m_wExCmd = wCommand ;					//m_wExCmd:只有在此处被赋值!
	return TRUE ;
}/*}}}*/

WORD CRtu104::GetCommand(  )
{/*{{{*/
	return m_wExCmd ;
}/*}}}*/

void CRtu104::YK_ErrorProcess( BYTE byAction , WORD wRelayNum , BYTE byActNum ,
		BYTE byActMark , WORD wReason )
{/*{{{*/
	if( wReason == 6 )
		wReason = 0x47 ;
	else if( wReason == 8 )
		wReason = 0x49 ;

	SetCommand( CMD_YK_ERROR ) ;
	SetYKData( byAction , wRelayNum , byActNum , byActMark , wReason ) ;
};/*}}}*/

/*
 *--------------------------------------------------------------------------------------
 *       Class:  CRtu104
 *      Method:  GetUnprocessBuf
 * Description:  获取没有被处理的 缓存区数据
 *       Input:
 *		Return:
 *--------------------------------------------------------------------------------------
 */
BOOL CRtu104::GetUnprocessBuf ( const BYTE *pBuf, const int iLen, void *pVoid )
{/*{{{*/
	int iTmpLen = iLen;
	int iPos = 0;
	UNPROCESSBUF *pData, tData;

	pData = (UNPROCESSBUF *)pVoid;

	if ( iTmpLen < 6 )
	{
		printf ( "IEC104 recvlen = %d\n", iTmpLen );
		return FALSE;
	}

	//获取一帧有效报文长度
	iTmpLen = pBuf[iPos + 1] + 2;
	while( iTmpLen >= 6 && iTmpLen <= 253 && 0x68 == pBuf[iPos])
	{
		if( iTmpLen > iLen - iPos )
			break;
		tData.len = iTmpLen;
		memcpy( tData.buf, pBuf, tData.len );
		m_UnProcessBuf.push( tData );

		iPos += ( iTmpLen + 2 );
		iTmpLen = pBuf[iPos + 1] + 2;
	}

	//将队列中最前端的取出 队列移动
	if( !m_UnProcessBuf.empty() )
	{
		tData = m_UnProcessBuf.front();
		pData->len = tData.len;
		memcpy(pData->buf, tData.buf, tData.len);
		m_UnProcessBuf.pop();
	}
	else
		return FALSE;

	return TRUE;
}		/* -----  end of method CRtu104::GetUnprocessBuf  ----- *//*}}}*/

BOOL CRtu104::GetDevCommState( )
{/*{{{*/
	if( m_bStartBit )
		return COM_DEV_NORMAL ;
	else
		return COM_DEV_ABNORMAL ;
}/*}}}*/

#if 0				//noted by cyz!
BOOL CRtu104::PackComStateMsg( BYTE * pBuf , BYTE &nlen , BYTE byLineNo , BYTE byDevAddr , BYTE byFlag )
{/*{{{*/
	if( !m_pMethod || nlen < 12 || pBuf == NULL )
		return FALSE ;

	BYTE byBusNum = m_pMethod->GetToTalBusNum() ;
	if( byLineNo >= byBusNum )
		return FALSE ;

	// BYTE byTotalDevNum = m_pMethod->GetDevNum( byLineNo ) ;
	// if( byDevAddr > byTotalDevNum )
	// return FALSE ;

	BYTE byIndex = 0 ;
	BYTE byType = 0 , byState = 0 ;

	REALTIME tm1;
	GetCurrentTime( &tm1 );

	WORD wPnt = 0 ;
	wPnt = COM_STATE_ADDR + byIndex;
	pBuf[nlen++] = LOBYTE(wPnt);
	pBuf[nlen++] = HIBYTE(wPnt);
	pBuf[nlen++] = 0;

	pBuf[nlen++]=LOBYTE(tm1.wMilliSec);
	pBuf[nlen++]=HIBYTE(tm1.wMilliSec);
	pBuf[nlen++]=tm1.wSecond;
	pBuf[nlen++]=tm1.wMinute;
	pBuf[nlen++]=tm1.wHour;
	pBuf[nlen++]=tm1.wDay;
	pBuf[nlen++]=tm1.wMonth;
	pBuf[nlen++]=( tm1.wYear ) % 100;

	if( byFlag == SEND_BUS_STATE )
	{
		byType=1;
		byState= GetComState( byLineNo ) ;
	}
	else if( byFlag == SEND_DEV_STATE )
	{
		byType=2;
		byState=GetDevCommState( byLineNo , byDevAddr ) ;
	}
	else
		return FALSE ;
	// printf ( "\n state is %d", byState );

	pBuf[nlen++]=byLineNo + 1;
	pBuf[nlen++]=byDevAddr;
	pBuf[nlen++]=byState;
	pBuf[nlen++]=byType;

	return TRUE ;
}/*}}}*/
#endif

//added by cyz!
BOOL CRtu104::PackComStateMsg( BYTE * pBuf , BYTE &nlen , BYTE byLineNo , BYTE byDevAddr , BYTE byFlag )
{/*{{{*/
	if( !m_pMethod || nlen < 12 || pBuf == NULL )
		return FALSE ;

	BYTE byBusNum = m_pMethod->GetToTalBusNum() ;
	if( byLineNo >= byBusNum )
		return FALSE ;

	// BYTE byTotalDevNum = m_pMethod->GetDevNum( byLineNo ) ;
	// if( byDevAddr > byTotalDevNum )
	// return FALSE ;

	BYTE byIndex = 0 ;
	BYTE byType = 0 , byState = 0 ;

	REALTIME tm1;
	GetCurrentTime( &tm1 );

	WORD wPnt = 0 ;
	wPnt = COM_STATE_ADDR + byIndex;
	pBuf[nlen++] = LOBYTE(wPnt);
	pBuf[nlen++] = HIBYTE(wPnt);
	pBuf[nlen++] = 0;

	pBuf[nlen++]=LOBYTE(tm1.wMilliSec);
	pBuf[nlen++]=HIBYTE(tm1.wMilliSec);
	pBuf[nlen++]=tm1.wSecond;
	pBuf[nlen++]=tm1.wMinute;
	pBuf[nlen++]=tm1.wHour;
	pBuf[nlen++]=tm1.wDay;
	pBuf[nlen++]=tm1.wMonth;
	pBuf[nlen++]=( tm1.wYear ) % 100;

	if(QueryAllDevStatus){
		if( byFlag == SEND_BUS_STATE )
		{
			byType=1;
			byState= GetComState( byLineNo ) ;
		}
		else if( byFlag == SEND_DEV_STATE )
		{
			byType=2;
			byState=GetDevCommState( byLineNo , byDevAddr ) ;

			//			vec_stat.push_back(byState);			//error, 因为分批传会导致上一帧最后一个通道的状态在follow帧被重传!
			WORD serialno = m_pMethod->GetSerialNo(byLineNo, byDevAddr);
			map_stat[serialno] = byState;
			//printf("********	serialno:%d	********\n", serialno);
		}
		else
			return FALSE ;
	}else{
		byState = deque_stat.front().stat;
		deque_stat.pop_front();
	}
	// printf ( "\n state is %d", byState );

	pBuf[nlen++]=byLineNo + 1;
	pBuf[nlen++]=byDevAddr;
	pBuf[nlen++]=byState;
	//printf("^^^^^^^^	%4X	%4X	%4X	^^^^^^^^\n", byLineNo+1, byDevAddr, byState);
	pBuf[nlen++]=byType;

	return TRUE ;
}/*}}}*/

//added by cyz!
BOOL CRtu104::PackComStateMsg( BYTE * pBuf , BYTE &nlen , WORD serialno , BYTE byFlag )
{/*{{{*/
	if( !m_pMethod || nlen < 12 || pBuf == NULL )
		return FALSE ;

	BYTE byLineNo = 0;
	WORD byDevAddr = 0;
	if(!m_pMethod->GetBusLineAndAddr(serialno, byLineNo, byDevAddr)){
		perror("Failed to GetBusLineAndAddr!");
		return FALSE;
	}

	BYTE byBusNum = m_pMethod->GetToTalBusNum() ;
	if( byLineNo >= byBusNum )
		return FALSE ;

	// BYTE byTotalDevNum = m_pMethod->GetDevNum( byLineNo ) ;
	// if( byDevAddr > byTotalDevNum )
	// return FALSE ;

	BYTE byIndex = 0 ;
	BYTE byState = 0 ;

	REALTIME tm1;
	GetCurrentTime( &tm1 );

	WORD wPnt = 0 ;
	wPnt = COM_STATE_ADDR + byIndex;
	pBuf[nlen++] = LOBYTE(wPnt);
	pBuf[nlen++] = HIBYTE(wPnt);
	pBuf[nlen++] = 0;

	pBuf[nlen++]=LOBYTE(tm1.wMilliSec);
	pBuf[nlen++]=HIBYTE(tm1.wMilliSec);
	pBuf[nlen++]=tm1.wSecond;
	pBuf[nlen++]=tm1.wMinute;
	pBuf[nlen++]=tm1.wHour;
	pBuf[nlen++]=tm1.wDay;
	pBuf[nlen++]=tm1.wMonth;
	pBuf[nlen++]=( tm1.wYear ) % 100;


	if(QueryAllDevStatus){
		if( byFlag == SEND_BUS_STATE )
		{
			byState= GetComState( byLineNo ) ;
		}
		else if( byFlag == SEND_DEV_STATE )
		{
			byState=GetDevCommState( byLineNo , byDevAddr ) ;

			//			vec_stat.push_back(byState);			//error, 因为分批传会导致上一帧最后一个通道的状态在follow帧被重传!
			map_stat[serialno] = byState;
		}
		else
			return FALSE ;
	}else{
		byState = deque_stat.front().stat;
		deque_stat.pop_front();
	}
	// printf ( "\n state is %d", byState );

	pBuf[nlen++]=byLineNo + 1;
	pBuf[nlen++]=byDevAddr;
	pBuf[nlen++]=byState;
	pBuf[nlen++]=2;

	return TRUE ;
}/*}}}*/

BOOL CRtu104::GetComState( BYTE byLineNo )
{/*{{{*/
	if( !m_pMethod )
		return COM_ABNORMAL ;

	PBUSMANAGER pBus = m_pMethod->GetBus( byLineNo ) ;
	if( !pBus )
		return COM_ABNORMAL ;

	CProtocol * pProtocol = pBus->m_Protocol ;
	if( !pProtocol )
		return COM_ABNORMAL ;

	//判断双机冗余机制
	BOOL bStatus = COM_ABNORMAL ;
	// if( CPublicMethod::IsHaveDDB() &&
	// 	( CPublicMethod::GetDDBSyncState() == STATUS_SLAVE ) &&
	// 	pProtocol->m_ProtoType == PROTOCO_GATHER
	// 	)
	// {
	// 	if( !CPublicMethod::GetDDBBusLinkStatus( byLineNo , bStatus ) )
	// 		bStatus = COM_ABNORMAL ;
	// }
	// else
	bStatus= m_pMethod->GetCommState( byLineNo );

	return bStatus  ;
}/*}}}*/

BOOL CRtu104::GetDevCommState( BYTE byLineNo , BYTE byAddr )
{/*{{{*/
	if( !m_pMethod )
		return COM_DEV_ABNORMAL ;

	PBUSMANAGER pBus = m_pMethod->GetBus( byLineNo ) ;
	if( !pBus )
		return COM_DEV_ABNORMAL ;

	CProtocol * pProtocol = pBus->m_Protocol ;
	if( !pProtocol )
		return COM_DEV_ABNORMAL ;

	//判断双机冗余机制
	BOOL bStatus = COM_DEV_ABNORMAL ;
	// if( CPublicMethod::IsHaveDDB() &&
	// 	( CPublicMethod::GetDDBSyncState() == STATUS_SLAVE ) &&
	// 	pProtocol->m_ProtoType == PROTOCO_GATHER
	// 	)
	// {
	// 	WORD wSerialNo = m_pMethod->GetSerialNo( byLineNo , byAddr ) ;
	// 	if( !CPublicMethod::GetDDBStnLinkStatus( wSerialNo , bStatus ) )
	// 		bStatus = COM_DEV_ABNORMAL ;
	// }
	// else
	bStatus=m_pMethod->GetDevCommState( byLineNo , byAddr );

	return bStatus ;
}/*}}}*/
