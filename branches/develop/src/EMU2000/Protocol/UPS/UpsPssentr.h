#ifndef _UPSPSSENTR_H
#define _UPSPSSENTR_H

#define UPSBAISEMSGFALSE (1)
#define UPSBAISEMSGTRUE (0)

//#include "../../share/CProtocol.h"
#include "CProtocol_UpsMaster.h"

class CUpsSentry : public CProtocol_UpsMaster
{
public:
	CUpsSentry();
	virtual ~CUpsSentry();

	BYTE SendFlag;
	BOOL m_bLinkStatus;
	BYTE m_bySendCount;

	virtual BOOL GetProtocolBuf( BYTE * buf , int &len , PBUSMSG pBusMsg = NULL ) ;
	virtual BOOL ProcessProtocolBuf( BYTE * buf , int len ) ;
	virtual BOOL Init( BYTE byLineNo  ) ;
	WORD GetChecksum(unsigned char *Buf, int len);

	virtual void TimerProc() ;
	//获得装置通讯状态
	virtual BOOL GetDevCommState( ) ;

};

#endif // !defined(AFX_PROTOCOL_UPSBAISE_H__DB4E4A83_510B_4232_A294_B1B4EE1AF4FD__INCLUDED_)

