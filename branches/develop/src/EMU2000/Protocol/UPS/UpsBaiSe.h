#ifndef _UPSBAISE_H
#define _UPSBAISE_H

#define UPSBAISEMSGFALSE (1)
#define UPSBAISEMSGTRUE (0)

//#include "../../share/CProtocol.h"
#include "CProtocol_UpsMaster.h"

class CUpsBaiSe : public CProtocol_UpsMaster
{
public:
	CUpsBaiSe();
	virtual ~CUpsBaiSe();

	BYTE SendFlag;
	BOOL m_bLinkStatus;
	BYTE m_bySendCount;


	virtual BOOL GetProtocolBuf( BYTE * buf , int &len , PBUSMSG pBusMsg = NULL ) ;
	virtual BOOL ProcessProtocolBuf( BYTE * buf , int len ) ;
	virtual BOOL Init( BYTE byLineNo  ) ;

	virtual void TimerProc() ;
	//获得装置通讯状态
	virtual BOOL GetDevCommState( ) ;

	BOOL GetYKBuffer( BYTE * buf , int &len , PBUSMSG pBusMsg );

	BOOL UPSBAISEQueryStatePack( BYTE * buf , int &len );

	BOOL UPSBAISEQueryCellStatePack( BYTE * buf , int &len );

	BOOL UPSBAISEQueryG2StatePack( BYTE * buf , int &len );

	BOOL UPSBAISEQueryG3StatePack( BYTE * buf , int &len );

	BOOL UPSBAISETestTenSecondPack( BYTE * buf , int &len );
	BOOL UPSBAISETestundervoltagePack( BYTE * buf , int &len );
	BOOL UPSBAISETestSomeMinutesPack( BYTE * buf , int &len );
	BOOL UPSBAISEControlBuzzerPack( BYTE * buf , int &len );
	BOOL UPSBAISEPowerOffPack( BYTE * buf , int &len );
	BOOL UPSBAISEPowerOffPowerOnPack( BYTE * buf , int &len );
	BOOL UPSBAISECancelPowerOffPack( BYTE * buf , int &len );
	BOOL UPSBAISECancelTestPack( BYTE * buf , int &len );

	BOOL UPSBAISEQueryStateDeal( BYTE * buf , int len );

	BOOL UPSBAISEQueryCellStateDeal( BYTE * buf , int len );

	BOOL UPSBAISEQueryG2StateDeal( BYTE * buf , int len );

	BOOL UPSBAISEQueryG3StateDeal( BYTE * buf , int len );
};

#endif // !defined(AFX_PROTOCOL_UPSBAISE_H__DB4E4A83_510B_4232_A294_B1B4EE1AF4FD__INCLUDED_)

