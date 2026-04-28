// Protocol_UPS.h: interface for the CProtocol_UPS class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_CPROTOCOL_MODBUSTCP_H__DB4E4A83_510B_4232_A294_B1B4EE1AF4FD__INCLUDED_)
#define AFX_CPROTOCOL_MODBUSTCP_H__DB4E4A83_510B_4232_A294_B1B4EE1AF4FD__INCLUDED_



#include "../../share/Rtu.h"
#include "../../share/CMethod.h"
#define	MODBUSTCPPREFIXFILENAME			"/mynand/config/ModBusTcp/"	

class CProtocol_ModBusTcp  : public CRtuBase
{
public:
	CProtocol_ModBusTcp();
	virtual ~CProtocol_ModBusTcp();

	virtual BOOL GetProtocolBuf( BYTE * buf , int &len , PBUSMSG pBusMsg = NULL ) ;
	virtual BOOL ProcessProtocolBuf( BYTE * buf , int len ) ;

	BOOL UPSQueryStatePack( BYTE * buf , int &len );

	BOOL UPSQueryStateDeal( BYTE * buf , int len );

	virtual BOOL Init( BYTE byLineNo  ) ;
	
	protected:
		BOOL GetDevData( ) ;
	protected:
		BOOL ProcessFileData( CProfile &profile );
		BOOL CreateModule( int iModule , int iSerialNo , WORD iAddr , char * sName , char * stplatePath ) ;
	//protected:
	char mt_sMasterAddr[ 24 ] ;
};

#endif // !defined(AFX_PROTOCOL_UPS_H__DB4E4A83_510B_4232_A294_B1B4EE1AF4FD__INCLUDED_)
