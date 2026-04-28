#ifndef CPROTOCOL_MODBUSTCP_H
#define CPROTOCOL_MODBUSTCP_H

#include "../../share/CProtocol.h"
#include "../../share/CMethod.h"

class CProtocol_ModBusTcp_Gather : public CProtocol
{
    public:
        CProtocol_ModBusTcp_Gather();
        virtual ~CProtocol_ModBusTcp_Gather();
		virtual BOOL GetProtocolBuf( BYTE * buf , int &len , PBUSMSG pBusMsg = NULL );
		virtual BOOL ProcessProtocolBuf( BYTE * buf , int len );
		virtual BOOL Init( BYTE byLineNo  ) ;
		virtual BOOL BroadCast( BYTE * buf , int &len ) ;
		virtual void TimerProc(){ return;  } 
	protected:
		BOOL GetDevData( ) ;
	protected:
		BOOL ProcessFileData( CProfile &profile );
		BOOL CreateModule( int iModule , int iSerialNo , WORD iAddr , char * sName , char * stplatePath ) ;

};

#endif // CPROTOCOL_MODBUS_H
