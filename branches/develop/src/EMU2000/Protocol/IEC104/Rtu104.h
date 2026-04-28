/******************************************************************************
 *  Rtu104.h: interface for the IEC104 protocol on Linux.
 *  Copyright (C): 2008 by houpeng
 ******************************************************************************/
#ifndef _RTU104_H_
#define _RTU104_H_

#include <queue>

#include "CProtocol_IEC104.h"
#include "../../share/gDataType.h"

#define RTU104_TX_BUF_SIZE  1024
#define RTU104_RX_BUF_SIZE  1024

#define RTU104MAX_AI_LEN    4096
#define RTU104MAX_DI_LEN    8192
#define RTU104MAX_PI_LEN    2048
#define RTU104MAX_DO_LEN    512

/******************************************************************************
 * CRtu104 Object
 */

#define SEND_BUS_STATE	1
#define SEND_DEV_STATE	2

class CRtu104 : public CProtocol_IEC104
{
	public:
		CRtu104();
		~CRtu104();

		/* Attributes */
	public:
		BOOL    m_bDataInit;
		BOOL    m_bStartBit;
		BOOL    m_bAllData;

		WORD    m_wCommand;
		WORD    m_wExCmd ;
		WORD    m_wStatus;
		BYTE    m_bySending;

	protected:
		BYTE  *m_pTX_Buf ;				//组织帧中转站!
		BYTE  * m_pRX_Buf ;
		int   m_nRXCount;
		int   m_nTXCount;
		int   m_RQ_WR_P;
		int   m_RQ_RD_P;

		WORD  m_wNumSend;   //发送计数
		WORD  m_wNumRecv;   //接受计数
		WORD  m_wNumRecBK; //已经接受的确认计数
		WORD  m_wNumAck;  //已经确认的发送计数
		WORD  m_wYkFlag;

	private:
		BYTE    m_byDataStyle;
		WORD    m_wDataIndex;

		WORD    m_wTimerCount;
		WORD    m_wErrTimer;
		WORD    m_wSendTimer;
		WORD    m_wRecvTimer;
		WORD    m_wIdleTimer;
		BYTE		m_byTimeFlag ;
		WORD    m_wDRFTimer;
		WORD    m_wPARTimer;
		time_t    m_t1 , m_t2 , m_t3 , m_t4 , m_YKTime;
		time_t    m_DevStateTime ; //上送装置状态时间
		BOOL	m_YkFlag ; //判断是否可以进行遥控
		WORD    m_wResendNum;
		BYTE    m_byToutNum;
		BYTE    m_byErrCode;
		BYTE		m_byDzOpt; //下发读定值 还是写定值
		BYTE    m_byDzNum; //下发定值数量
		BYTE    m_byDzStartNo; //定值其实序号
		DZ_DATA m_ReadzData[100]; //将消息传过来的定值保存

		float     m_wAIBuf[RTU104MAX_AI_LEN];	//保存遥测，该遥测用来和新数据比较
		QWORD   m_dwPIBuf[RTU104MAX_PI_LEN];	//同上，保存遥脉!
		std::queue<UNPROCESSBUF> m_UnProcessBuf;

		BYTE	m_byDIbuf[ RTU104MAX_DI_LEN ] ;			//同上，保存遥信!

		void DzProcess(BYTE * pBuf);
		void DzRead(BYTE byBusNo, BYTE byAddr, BYTE byNum, BYTE * pBuf);
		void DzWrite(BYTE byBusNo, BYTE byAddr, BYTE byNum, BYTE * pBuf);
protected:
		BYTE CalBch(BYTE* pTBuf, int len);
		int ProcessDz(PBUSMSG pBusMsg);
		int AckMessageProc(BYTE* pDataBuf, int nLen);
		int InfoMessageProc(BYTE* pDataBuf, int nLen);
		int	DealBusMsgInfo(PBUSMSG pBusMsg);

		int RelayCmdProc( BYTE byTypeID , BYTE byReason , WORD wInfoAddr , BYTE byVal );
		int RelaySelectProc(WORD wStn, WORD wCtrlNum, BYTE byStatus);
		int RelayExecuteProc(WORD wStn, WORD wCtrlNum, BYTE byStatus);
		int RelayCancelProc(WORD wStn, WORD wCtrlNum, BYTE byStatus);
		int	GroupReqProc(BYTE* pDataBuf, int nLen);
		int	PulseReqProc(BYTE* pDataBuf, int nLen);
		BOOL SysClockProc(BYTE* pDataBuf, int nLen);
		int	ResetProc(BYTE* pDataBuf, int nLen);

		int	AckFrame(void);
		int TestFrameAck(void);
		int	StartDataAck(void);
		int	StopDataAck(void);
		int	AllDataEcho(WORD wUnitAddr, BYTE byReason);
		int ReqPulseEcho(WORD wUnitAddr, BYTE byReason, BYTE byQCC);

		int LoadAIEFrame21(WORD wUnitAddr);
		int LoadAIEFrame13(WORD wUnitAddr);
		int LoadDIEFrame(WORD wUnitAddr);
		int LoadSOEFrame_30(WORD wUnitAddr);
		int LoadSOEFrame_02(WORD wUnitAddr);

		int LoadAnalogData21(WORD wUnitAddr);
		int LoadAnalogData13(WORD wUnitAddr);
		int LoadDigitalData(WORD wUnitAddr);
		int LoadPulseData(WORD wUnitAddr);
		int LoadAllData(WORD wUnitAddr);
		int LoadAnalogGroup(BYTE byGroup, WORD wUnitAddr, BYTE byReason);
		int LoadDigitalGroup(BYTE byGroup, WORD wUnitAddr, BYTE byReason);
		int LoadPulseGroup(BYTE byGroup, WORD wUnitAddr, BYTE byReason);
		int LoadRelayEchoFrame(WORD wUnitAddr, BYTE byType, BYTE byReason, WORD wIndexNum, BYTE byCmd);
		int SysClockConfirm( WORD wAddr ) ;
		BOOL GetStartDt( );
		WORD GetSeqNo( BYTE * pointer );
		BOOL GetSendRecvNo( WORD &wSend , WORD &wRecv ) ;
		BOOL ProcSendNo( WORD wAckNo ) ;
		BOOL ProcRecvNo( WORD wRecNo ) ;

		virtual BOOL ReSetState( ) ;
		void ProcessIEC104Time( );
		BYTE GetTimeFlag( ) ;
		BOOL SetTTimer( BYTE byTime , BYTE byState );
		int TestSend( ) ;
		void SetTransIndex( WORD Index , WORD DataStyle );
		BYTE GetS_DCO( BYTE byTypeID , BYTE byVal ) ;
		void GetYkData( BYTE &byAction , WORD  &wRelayNum , BYTE & byActNum ,
				BYTE & byActMark , WORD & wSelectTime ) ;
		void SetYKData( BYTE byAction , WORD wRelayNum , BYTE byActNum ,
				BYTE byActMark , WORD wSelectTime ) ;
		void SetIEC104Time( time_t *tTime , BYTE byState , BYTE byBeginT , BYTE byEndT );
		void YK_ErrorProcess( BYTE byAction , WORD wRelayNum , BYTE byActNum ,
				BYTE byActMark , WORD wSelectTime ) ;
		void YK_PreSet( BOOL bPreSetFlag ) ;
		BOOL YK_IsCanSend(  ) ;
		void YK_RtnConfirm( BOOL bFlag ) ;
		void YK_Rtn( int &len ) ;

		int  Resend_Proc( void );
		void RecvToutProc( void );
		int  StartedProcess( WORD wAddr ) ;
		int PreProcess( WORD wAddr ) ;
		int LoadReadDzEchoFrame();
		int LoadDzEchoFrame();
		int ChangeDataProcess(WORD wAddr);
		BOOL IsCanSend( ) ;
		virtual BOOL GetDevCommState( ) ;
		virtual int ComState_Message( WORD wUnitAddr ) { return 0 ; }
		BOOL PackComStateMsg( BYTE * pBuf , BYTE &nlen , BYTE byLineNo , BYTE byDevAddr , BYTE byFlag ) ;
		BOOL PackComStateMsg( BYTE * pBuf , BYTE &, WORD, BYTE byFlag );
		BOOL GetComState( BYTE byLineNo );
		BOOL GetDevCommState( BYTE byLineNo , BYTE byAddr ) ;

		BOOL QueryAllDevStatus;				//by cyz!	标志是否求取全部的设备状态，当第一次设备状态被轮询之后即永远置为假!
		/* Implementation */
	public:
		virtual char* ProName(){return (char *)"IEC104";}
		virtual BOOL InitRtuBase();
		void writeLog(char * pContent, int len);
		BOOL LoadRtuMessage(BYTE * pBuf, int &len);
		virtual void    RtuCommandProc( BYTE* pRecvBuf, int nLen );
		virtual void    TimerProc();

		virtual  int  GetRealVal(BYTE byType, WORD wPnt, void *v);

		virtual BOOL WriteCIVal( WORD wSerialNo , WORD wPnt, float fVal){ return FALSE ;}
		virtual BOOL WriteAIVal(WORD wSerialNo ,WORD wPnt, float fVal) ;
		virtual BOOL WriteDIVal(WORD wSerialNo ,WORD wPnt, WORD wVal) ;
		virtual BOOL WritePIVal(WORD wSerialNo ,WORD wPnt, QWORD dwVal);
		virtual BOOL WriteSOEInfo( WORD wSerialNo ,WORD wPnt, WORD wVal, LONG lTime, WORD wMiSecond) ;

		virtual  void RelayEchoProc(BYTE byCommand, WORD wIndex, BYTE byResult);
		virtual  int TransMessage( BYTE byType, BYTE* pBuf, int nLen );
		virtual  int GetCommObjProp(COMMOBJ_PROP* pObjProp);

		void TaskProcHandle(void);
		virtual BOOL Init( BYTE byLineNo );
		virtual BOOL GetProtocolBuf( BYTE * buf , int &len , PBUSMSG pBusMsg = NULL ) ;
		virtual BOOL ProcessProtocolBuf( BYTE * buf , int len ) ;
		virtual BOOL GetUnprocessBuf ( const BYTE *pBuf, const int iLen, void *pVoid );

		BOOL SetCommand( WORD wCommand ) ;
		WORD GetCommand(  ) ;

		template< typename Type > void InitBuffer( Type *tData , int size );

};
/******************************************************************************/

#endif
