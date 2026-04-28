// Protocol_UPS.h: interface for the CProtocol_ESDCMMI class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_CPROTOCOL_ESDCMMI_H__DB4E4A83_510B_4232_A294_B1B4EE1AF4FD__INCLUDED_)
#define AFX_CPROTOCOL_ESDCMMI_H__DB4E4A83_510B_4232_A294_B1B4EE1AF4FD__INCLUDED_



#include "../../share/Rtu.h"
#include "../../share/CMethod.h"
#define	ESDCMMIPREFIXFILENAME			"/mynand/config/ESDCMMI/"	

class CProtocol_ESDCMMI  : public CRtuBase
{
public:
	CProtocol_ESDCMMI();
	virtual ~CProtocol_ESDCMMI();
	virtual BYTE ESDCMMI_GetCRC(BYTE  *buf, int len);
	virtual BOOL Init( BYTE byLineNo  ) ;
	
	protected:
		BOOL GetDevData( ) ;
	protected:
		BOOL ProcessFileData( CProfile &profile );
		BOOL CreateModule( int iModule , int iSerialNo , WORD iAddr , char * sName , char * stplatePath ) ;
	//protected:
	char mt_sMasterAddr[ 24 ] ;
};

#endif // !defined(AFX_PROTOCOL_ESDCMMI_H__DB4E4A83_510B_4232_A294_B1B4EE1AF4FD__INCLUDED_)
