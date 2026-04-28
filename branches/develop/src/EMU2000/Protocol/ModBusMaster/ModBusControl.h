#if !defined(ModBusMaster_)
#define ModBusMaster_

#define ModBusMasterMSGFALSE (1)
#define ModBusMasterMSGTRUE (0)

//#include "../../share/CProtocol.h"
#include "CProtocol_ModBusMaster.h"

class CModBusControl  : public CProtocol_ModBusMaster
{
public:
	CModBusControl();
	virtual ~CModBusControl();
	
	BYTE m_byPortStatus;
	int m_wErrorTimer;

	virtual BOOL GetProtocolBuf( BYTE * buf , int &len , PBUSMSG pBusMsg = NULL ) ;
	virtual BOOL ProcessProtocolBuf( BYTE * buf , int len ) ;
	virtual BOOL Init( BYTE byLineNo  ) ;
	
	virtual void TimerProc() ;
	//获得装置通讯状态 by	zhanghg 2014-9-23
	virtual BOOL GetDevCommState( ) ;

	BOOL DealRecvMsg( BYTE * buf , int len );
};

#endif // !defined(AFX_PROTOCOL_ModBusMaster_H__DB4E4A83_510B_4232_A294_B1B4EE1AF4FD__INCLUDED_)

