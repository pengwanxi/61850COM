// Protocol_ESD_ModBusSlave.h: interface for the CProtocol_ESD_ModBusSlave class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_PROTOCOL_ESD_MODBUSSLAVE_H__5F0D50B1_834A_44A4_AA42_D89ACB8F03E3__INCLUDED_)
#define AFX_PROTOCOL_ESD_MODBUSSLAVE_H__5F0D50B1_834A_44A4_AA42_D89ACB8F03E3__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "Protocol_ModBusSlave.h"
#include "../../share/Rtu.h"
#include "../../share/CMethod.h"
#include "../../share/CProtocol.h"

#include <vector>
using namespace std ;
#define MS_TX_BUF_SIZE  1024
#define MS_RX_BUF_SIZE  1024

#define MSMAX_YC_LEN    6800
#define MSMAX_YX_LEN    10000
#define MSMAX_YM_LEN    1024
#define MSMAX_YK_LEN    10000

typedef struct _MBS_BUSDEV
{
	BYTE busNo ;
	WORD wAddr ;
	_MBS_BUSDEV( )
	{
		busNo = 0 ;
		wAddr = 0 ;
	}
}MBS_BUSDEV , *PMBS_BUSDEV ;

class CProtocol_ESD_ModBusSlave  : public CRtuBase
{
public:
	CProtocol_ESD_ModBusSlave();
	virtual ~CProtocol_ESD_ModBusSlave();
	virtual BOOL Init( BYTE byLineNo ) ;
	virtual BOOL InitRtuBase() ;
	BOOL WriteAIVal(WORD wSerialNo, WORD wPnt, float fVal );
	BOOL WriteDIVal(WORD wSerialNo, WORD wPnt, WORD wVal);
	BOOL GetProtocolBuf( BYTE * buf , int &len , PBUSMSG pBusMsg  );
	BOOL ProcessProtocolBuf( BYTE * pbuf , int len );
	void TimerProc();
	BOOL yxProcess( BYTE byAddr, BYTE *pBuf, WORD len) ;
	BOOL ycProcess( BYTE byAddr, BYTE *pBuf, WORD len );
	BOOL ErrMessage( BYTE byAddr, BYTE byFuncCode, BYTE byErrCode, BYTE * pBuf , int &len );
	BOOL yxMessage( BYTE byAddr , BYTE * buf , int &len );
	BOOL ycMessage( BYTE byAddr , BYTE * pbuf , int &len );
	BOOL SetErrorMsg( BYTE byFuncCode, BYTE byErrCode );
	int  GetRealVal(BYTE byType, WORD wPnt, void *v);
	BOOL ymMessage( BYTE byAddr , BYTE * buf , int &len );
	BOOL ymProcess( BYTE byAddr, BYTE *pBuf, WORD len );
	BOOL dataProcess( BYTE byAddr, BYTE *pBuf, WORD len );
	void SetFlag( DWORD dwVal , BOOL bFlag = TRUE ) ;
	BOOL GetFlag( DWORD dwVal );
	BOOL IsCanSend( );
	BOOL yxDataProcess( BYTE byAddr , BYTE *pBuf , WORD len );
	BOOL InitDevState( );
	BOOL yxDevStateProcess( BYTE byAddr ,BYTE *pBuf , WORD len ) ;
	BOOL yxDevState( BYTE byAddr , BYTE * buf , int &len );
	BOOL busRSProcess( BYTE byAddr , BYTE * pbuf , WORD len  );

	BOOL GetDevState( WORD wYxNo , BYTE *byVal );
	virtual BOOL ykProcess(BYTE byAddr, BYTE *pBuf, WORD len) ;
	virtual BOOL ykMessage( BYTE byAddr , BYTE * pbuf , int &len , PBUSMSG pBusMsg );
	BOOL busRSMessage( BYTE byAddr , BYTE * pbuf , int &len ) ;
	WORD GetCRC(BYTE *pBuf, WORD nLen);

public:
	BYTE m_RealType ;
	WORD m_wRegAddr;
	WORD m_wRegNum;
	WORD m_wYkAddr ;
	WORD m_wYkNum ;
	WORD m_Action ;
	BYTE m_FucCode ;
	BYTE m_FuncError ;
	DWORD m_dwSendFlag ;

	float     m_wYcBuf[MSMAX_YC_LEN];
	BYTE	m_byYxBuf[ MSMAX_YX_LEN ] ;
	std::vector< PMBS_BUSDEV > m_busDevStateArray;
};

#endif // !defined(AFX_PROTOCOL_ESD_MODBUSSLAVE_H__5F0D50B1_834A_44A4_AA42_D89ACB8F03E3__INCLUDED_)
