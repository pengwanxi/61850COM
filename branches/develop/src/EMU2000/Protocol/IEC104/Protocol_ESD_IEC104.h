// Protocol_ESD_IEC104.h: interface for the CProtocol_ESD_IEC104 class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_PROTOCOL_ESD_IEC104_H__B25ED719_D5B5_4FF3_9241_76EE4FEC5256__INCLUDED_)
#define AFX_PROTOCOL_ESD_IEC104_H__B25ED719_D5B5_4FF3_9241_76EE4FEC5256__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "Rtu104.h"

class CProtocol_ESD_IEC104  : public CRtu104
{
	public:
		CProtocol_ESD_IEC104();
		virtual ~CProtocol_ESD_IEC104();

		virtual int ComState_Message( WORD wUnitAddr ) ;
		virtual BOOL ReSetState( ) ;
	private:
		BYTE byStaticLineNo  ;
		BYTE byStaticDevAddr ;
};

#endif // !defined(AFX_PROTOCOL_ESD_IEC104_H__B25ED719_D5B5_4FF3_9241_76EE4FEC5256__INCLUDED_)
