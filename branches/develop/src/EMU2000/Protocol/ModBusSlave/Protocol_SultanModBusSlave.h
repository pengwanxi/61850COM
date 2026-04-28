// SultanModBusSlave.h: interface for the CSultanModBusSlave class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_SULTANMODBUSSLAVE_H__26C2CB04_A9B7_4BF6_82DD_21BE65F20664__INCLUDED_)
#define AFX_SULTANMODBUSSLAVE_H__26C2CB04_A9B7_4BF6_82DD_21BE65F20664__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
#include "Protocol_ESD_ModBusSlave.h"

class CProtocol_SultanModBusSlave  : public CProtocol_ESD_ModBusSlave
{
public:
	CProtocol_SultanModBusSlave();
	virtual ~CProtocol_SultanModBusSlave();

	virtual BOOL ykProcess(BYTE byAddr, BYTE *pBuf, WORD len) ;
	virtual BOOL ykMessage( BYTE byAddr , BYTE * pbuf , int &len , PBUSMSG pBusMsg );
};

#endif // !defined(AFX_SULTANMODBUSSLAVE_H__26C2CB04_A9B7_4BF6_82DD_21BE65F20664__INCLUDED_)
