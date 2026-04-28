#ifndef CPROTOCOL_CDT_H
#define CPROTOCOL_CDT_H

#include "../../share/CProtocol.h"
#include "../../share/CMethod.h"
class CProtocol_Cdt : public CProtocol
{
	public:
		CProtocol_Cdt();
		virtual ~CProtocol_Cdt();
		virtual BYTE GetCrc(BYTE *);
		virtual BOOL GetProtocolBuf( BYTE * buf , int &len , PBUSMSG pBusMsg = NULL );
		virtual BOOL ProcessProtocolBuf( BYTE * buf , int len );
		virtual BOOL Init(BYTE byLineNo  );
		virtual BOOL BroadCast( BYTE * buf , int &len );
		virtual void TimerProc(){ return;  } 
	protected:
		BOOL GetDevData( );
	protected:
		BOOL ProcessFileData( CProfile &profile );
		BOOL CreateModule( int iModule , int iSerialNo , WORD iAddr , char * sName , char * stplatePath ) ;
};

#endif // CPROTOCOL_CDT_H
